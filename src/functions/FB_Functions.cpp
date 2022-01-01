/**
 * Google's Cloud Functions class, Functions.cpp version 1.1.8
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

#ifndef _FB_FUNCTIONS_CPP_
#define _FB_FUNCTIONS_CPP_
#include "FB_Functions.h"

FB_Functions::FB_Functions()
{
}

FB_Functions::~FB_Functions()
{
}

bool FB_Functions::mCallFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *data)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_call;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;

    MBSTRING find;
    find.appendP(fb_esp_pgm_str_3);
    MBSTRING replace;
    replace.appendP(fb_esp_pgm_str_570);
    req.payload = data;
    ut->replaceAll(req.payload, find, replace);

   
    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();
    fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_135), req.payload.c_str());
    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();
    return sendRequest(fbdo, &req);
}

void FB_Functions::addCreationTask(FirebaseData *fbdo, FunctionsConfig *config, bool patch, fb_esp_functions_creation_step step, fb_esp_functions_creation_step nextStep, FunctionsOperationCallback callback, FunctionsOperationStatusInfo *statusInfo)
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
    fbdo->_ss.long_running_task++;

    _creation_task_enable = true;

    if (_deployTasks.size() == 1)
    {

#if defined(ESP32)
        runDeployTask(pgm2Str(fb_esp_pgm_str_475));
#elif defined(ESP8266)
        runDeployTask();
#endif
    }
}

bool FB_Functions::createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationStatusInfo *statusInfo)
{
    return createFunctionInt(fbdo, config->_name.c_str(), config, false, NULL, statusInfo);
}

bool FB_Functions::createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationCallback callback)
{
    return createFunctionInt(fbdo, config->_name.c_str(), config, false, callback, nullptr);
}

bool FB_Functions::mPatchFunction(FirebaseData *fbdo, const char *functionId, FunctionsConfig *patchData)
{
    return createFunctionInt(fbdo, functionId, patchData, true, NULL, nullptr);
}

bool FB_Functions::createFunctionInt(FirebaseData *fbdo, const char *functionId, FunctionsConfig *config, bool patch, FunctionsOperationCallback cb, FunctionsOperationStatusInfo *info)
{

    if (patch)
    {
        fbdo->_ss.cfn.cbInfo.functionId = functionId;
    }
    else
    {
        if (strlen(functionId) > 0)
            fbdo->_ss.cfn.cbInfo.functionId = functionId;
        else if (config->_entryPoint.length() > 0)
            fbdo->_ss.cfn.cbInfo.functionId = config->_entryPoint;
    }

    if (config->_sourceType == functions_sources_type_local_archive || config->_sourceType == functions_sources_type_flash_data)
    {
        if (config->_sourceType == functions_sources_type_local_archive)
        {
            if (config->_uploadArchiveStorageType == mem_storage_type_sd)
            {
                if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                    sendCallback(fbdo, fb_esp_functions_operation_status_error, fbdo->errorReason().c_str(), cb, info);
                    return false;
                }
#if defined SD_FS
                if (!SD_FS.exists(config->_uploadArchiveFile.c_str()))
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_ARCHIVE_NOT_FOUND;
                    sendCallback(fbdo, fb_esp_functions_operation_status_error, fbdo->errorReason().c_str(), cb, info);
                    return false;
                }
                Signer.getCfg()->_int.fb_file = SD_FS.open(config->_uploadArchiveFile.c_str(), FILE_READ);
#else
                return false;
#endif
            }
            else if (config->_uploadArchiveStorageType == mem_storage_type_flash)
            {

#if defined FLASH_FS

                if (!Signer.getCfg()->_int.fb_flash_rdy)
                    ut->flashTest();

                if (!Signer.getCfg()->_int.fb_flash_rdy || !FLASH_FS.exists(config->_uploadArchiveFile.c_str()))
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_ARCHIVE_NOT_FOUND;
                    sendCallback(fbdo, fb_esp_functions_operation_status_error, fbdo->errorReason().c_str(), cb, info);
                    return false;
                }

                Signer.getCfg()->_int.fb_file = FLASH_FS.open(config->_uploadArchiveFile.c_str(), "r");
#else
                return false;
#endif
            }

            if (!Signer.getCfg()->_int.fb_file)
            {
                fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                sendCallback(fbdo, fb_esp_functions_operation_status_error, fbdo->errorReason().c_str(), cb, info);
                return false;
            }
        }

        addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_gen_upload_url, fb_esp_functions_creation_step_upload_zip_file, cb, info);
        return true;
    }
    else if (config->_sourceType == functions_sources_type_storage_bucket_sources)
    {

        addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_upload_source_files, fb_esp_functions_creation_step_deploy, cb, info);
        return true;
    }
    else
    {
        bool ret = false;
        fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
        sendCallback(fbdo, fbdo->_ss.cfn.cbInfo.status, "", cb, info);
        ret = deploy(fbdo, functionId, config, patch);
        fbdo->_ss.cfn.cbInfo.triggerUrl = config->_httpsTriggerUrl;

        if (ret && config->_policy)
            addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_polling_status, fb_esp_functions_creation_step_set_iam_policy, cb, info);
        else if (ret)
            addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_polling_status, fb_esp_functions_creation_step_idle, cb, info);
        return ret;
    }

    return true;
}

bool FB_Functions::uploadSources(FirebaseData *fbdo, FunctionsConfig *config)
{
    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();

    
    fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_387), config->_projectId.c_str());

    fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_409), config->_locationId.c_str());
   

    fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_453), pgm2Str(fb_esp_pgm_str_456));
   
    fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_454), Signer.getToken());
    
    fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_455), config->_bucketSourcesPath.c_str());

    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_upload_bucket_sources;

    req.payload = fbdo->_ss.jsonPtr->raw();

    fbdo->_ss.jsonPtr->clear();

    req.host += config->_locationId.c_str();
    req.host.appendP(fb_esp_pgm_str_397);
    req.host += config->_projectId.c_str();
    req.host.appendP(fb_esp_pgm_str_398);

    req.uri.appendP(fb_esp_pgm_str_1);
    req.uri.appendP(fb_esp_pgm_str_452);

    return sendRequest(fbdo, &req);
}

bool FB_Functions::deploy(FirebaseData *fbdo, const char *functionId, FunctionsConfig *config, bool patch)
{

    struct fb_esp_functions_req_t req;
    if (patch)
    {
        req.requestType = fb_esp_functions_request_type_patch;
        req.updateMask = &config->_updateMask;
    }
    else
        req.requestType = fb_esp_functions_request_type_create;

    req.projectId = config->_projectId.c_str();
    req.locationId = config->_locationId.c_str();
    req.functionId = functionId;

    MBSTRING path;
    MBSTRING t;

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();

    path.appendP(fb_esp_pgm_str_387, true);
    fbdo->_ss.jsonPtr->add(path.c_str(), config->_projectId.c_str());

    path.appendP(fb_esp_pgm_str_388, true);
    t.appendP(fb_esp_pgm_str_112);
    t += Signer.getCfg()->database_url.c_str();
    fbdo->_ss.jsonPtr->add(path.c_str(), t.c_str());

    if (config->_bucketId.length() > 0)
    {
        path.appendP(fb_esp_pgm_str_389, true);
        fbdo->_ss.jsonPtr->add(path.c_str(), config->_bucketId.c_str());
    }

    if (config->_locationId.length() > 0)
    {
        path.appendP(fb_esp_pgm_str_390, true);
        fbdo->_ss.jsonPtr->add(path.c_str(), config->_locationId.c_str());
    }

    t.clear();
    MBSTRING str = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();

    ut->replaceAll(str, pgm2Str(fb_esp_pgm_str_3), pgm2Str(fb_esp_pgm_str_396));

    t.appendP(fb_esp_pgm_str_374, true);
    t.appendP(fb_esp_pgm_str_1);
    t.appendP(fb_esp_pgm_str_386);

    config->_funcCfg.set(t.c_str(), str.c_str());

    config->_httpsTriggerUrl.clear();

    MBSTRING().swap(config->_httpsTriggerUrl);
    MBSTRING().swap(str);

    if (config->_triggerType == fb_esp_functions_trigger_type_https)
    {
        t.clear();
        t.appendP(fb_esp_pgm_str_112);
        t += config->_locationId.c_str();
        t.appendP(fb_esp_pgm_str_397);
        t += config->_projectId.c_str();
        t.appendP(fb_esp_pgm_str_398);
        t.appendP(fb_esp_pgm_str_1);
        t += config->_entryPoint;
        config->_funcCfg.set(pgm2Str(fb_esp_pgm_str_384), t.c_str());
        config->_httpsTriggerUrl = t;
        MBSTRING().swap(t);
    }

    req.payload = config->_funcCfg.raw();
    config->_funcCfg.clear();

    return sendRequest(fbdo, &req);
}

void FB_Functions::sendCallback(FirebaseData *fbdo, fb_esp_functions_operation_status status, const char *message, FunctionsOperationCallback cb, FunctionsOperationStatusInfo *info)
{
    if (fbdo->_ss.cfn.last_status == status)
        return;

    fbdo->_ss.cfn.last_status = status;

    fbdo->_ss.cfn.cbInfo.status = status;
    fbdo->_ss.cfn.cbInfo.errorMsg = message;

    if (cb)
        cb(fbdo->_ss.cfn.cbInfo);

    if (info)
    {
        info->errorMsg = fbdo->_ss.cfn.cbInfo.errorMsg;
        info->status = fbdo->_ss.cfn.cbInfo.status;
    }
}

bool FB_Functions::mSetIamPolicy(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, PolicyBuilder *policy, const char *updateMask)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_set_iam_policy;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();

    if (policy)
    {
        static FirebaseJson js;
        js.clear();
        fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_399), policy->json);
    }

    if (strlen(updateMask) > 0)
    {
        fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_400), updateMask);
    }

    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetIamPolicy(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *version)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_get_iam_policy;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;
    req.policyVersion = version;
    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_get;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;
    _function_status = fb_esp_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;

    bool ret = sendRequest(fbdo, &req);
    if (ret)
    {
        if (!fbdo->_ss.jsonPtr)
            fbdo->_ss.jsonPtr = new FirebaseJson();

        if (!fbdo->_ss.dataPtr)
            fbdo->_ss.dataPtr = new FirebaseJsonData();

        fbdo->_ss.jsonPtr->clear();
        fbdo->_ss.jsonPtr->setJsonData(fbdo->_ss.cfn.payload.c_str());
        fbdo->_ss.jsonPtr->get(*fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_419));

        if (fbdo->_ss.dataPtr->success)
        {
            MBSTRING s;
            s.appendP(fb_esp_pgm_str_420, true);
            if (strcmp_P(fbdo->_ss.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;

            s.appendP(fb_esp_pgm_str_421, true);
            if (strcmp_P(fbdo->_ss.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_ACTIVE;

            s.appendP(fb_esp_pgm_str_422, true);
            if (strcmp_P(fbdo->_ss.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_OFFLINE;

            s.appendP(fb_esp_pgm_str_423, true);
            if (strcmp_P(fbdo->_ss.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_DEPLOY_IN_PROGRESS;

            s.appendP(fb_esp_pgm_str_424, true);
            if (strcmp_P(fbdo->_ss.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_DELETE_IN_PROGRESS;

            s.appendP(fb_esp_pgm_str_425, true);
            if (strcmp_P(fbdo->_ss.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_UNKNOWN;
            MBSTRING().swap(s);
        }
    }

    return ret;
}

bool FB_Functions::mListFunctions(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *pageSize, const char *pageToken)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_list;
    req.projectId = projectId;
    req.locationId = locationId;
    req.pageSize = atoi(pageSize);
    req.pageToken = pageToken;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mListOperations(FirebaseData *fbdo, const char *filter, const char *pageSize, const char *pageToken)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_list_operations;
    req.filter = filter;
    req.pageSize = atoi(pageSize);
    req.pageToken = pageToken;

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mDeleteFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_delete;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateDownloadUrl(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *versionId)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_gen_download_url;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;
    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();

    if (strlen(versionId) > 0)
    {
        req.versionId = atoi(versionId);
        fbdo->_ss.jsonPtr->add(pgm2Str(fb_esp_pgm_str_437), (int)versionId);
    }

    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateUploadUrl(FirebaseData *fbdo, const char *projectId, const char *locationId)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_gen_upload_url;
    req.projectId = projectId;
    req.locationId = locationId;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::uploadFile(FirebaseData *fbdo, const char *uploadUrl, const char *filePath, fb_esp_mem_storage_type storageType)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_upload;

    if (strlen(filePath) > 0)
    {
        if (filePath[0] != '/')
            req.filePath.appendP(fb_esp_pgm_str_1);
    }

    req.filePath += filePath;
    req.storageType = storageType;

    struct fb_esp_url_info_t info;
    ut->getUrlInfo(uploadUrl, info);
    req.host = info.host;
    req.uri.appendP(fb_esp_pgm_str_1);
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
    ut->getUrlInfo(uploadUrl, info);
    req.host = info.host;
    req.uri.appendP(fb_esp_pgm_str_1);
    req.uri += info.uri;
    return sendRequest(fbdo, &req);
}

void FB_Functions::begin(UtilsClass *u)
{
    ut = u;
}

bool FB_Functions::sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req)
{
    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

#ifdef ENABLE_RTDB
    if (fbdo->_ss.rtdb.pause)
        return true;
#endif

    if (!fbdo->reconnect())
        return false;

    if (!Signer.tokenReady())
        return false;

    if (Signer.getCfg()->_int.fb_processing)
        return false;

    Signer.getCfg()->_int.fb_processing = true;

    fbdo->clear();

    connect(fbdo, req->host.c_str());

    return functions_sendRequest(fbdo, req);
}

bool FB_Functions::functions_sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req)
{
    bool post = false;
    fbdo->_ss.cfn.requestType = req->requestType;

    MBSTRING header;
    if (req->requestType == fb_esp_functions_request_type_get_iam_policy || req->requestType == fb_esp_functions_request_type_list_operations || req->requestType == fb_esp_functions_request_type_get || req->requestType == fb_esp_functions_request_type_get_iam_policy || req->requestType == fb_esp_functions_request_type_list)
        header.appendP(fb_esp_pgm_str_25);
    else if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources || req->requestType == fb_esp_functions_request_type_call || req->requestType == fb_esp_functions_request_type_create || req->requestType == fb_esp_functions_request_type_gen_download_url || req->requestType == fb_esp_functions_request_type_gen_upload_url || req->requestType == fb_esp_functions_request_type_set_iam_policy || req->requestType == fb_esp_functions_request_type_test_iam_policy)
    {
        header.appendP(fb_esp_pgm_str_24);
        post = true;
    }
    else if (req->requestType == fb_esp_functions_request_type_patch)
    {
        header.appendP(fb_esp_pgm_str_26);
        post = true;
    }
    else if (req->requestType == fb_esp_functions_request_type_delete)
        header.appendP(fb_esp_pgm_str_27);
    else if (req->requestType == fb_esp_functions_request_type_upload || req->requestType == fb_esp_functions_request_type_pgm_upload)
        header.appendP(fb_esp_pgm_str_23);

    header.appendP(fb_esp_pgm_str_6);

    if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources || req->requestType == fb_esp_functions_request_type_upload || req->requestType == fb_esp_functions_request_type_pgm_upload)
        header += req->uri;
    else if (req->requestType == fb_esp_functions_request_type_list_operations)
    {
        header.appendP(fb_esp_pgm_str_426);
        bool hasParam = false;

        if (req->filter.length() > 0)
        {
            hasParam = true;
            header.appendP(fb_esp_pgm_str_173);
            header.appendP(fb_esp_pgm_str_427);
            header += req->filter;
        }

        if (req->pageSize > 0)
        {
            if (hasParam)
                header.appendP(fb_esp_pgm_str_172);
            else
                header.appendP(fb_esp_pgm_str_173);
            header.appendP(fb_esp_pgm_str_357);
            header.appendP(fb_esp_pgm_str_361);
            header += req->pageSize;
            hasParam = true;
        }

        if (req->pageToken.length() > 0)
        {
            if (hasParam)
                header.appendP(fb_esp_pgm_str_172);
            else
                header.appendP(fb_esp_pgm_str_173);
            header.appendP(fb_esp_pgm_str_358);
            header.appendP(fb_esp_pgm_str_361);
            header += req->pageToken;
            hasParam = true;
        }
    }
    else
    {
        header.appendP(fb_esp_pgm_str_326);

        if (req->projectId.length() == 0)
            header += Signer.getCfg()->service_account.data.project_id;
        else
            header += req->projectId;

        header.appendP(fb_esp_pgm_str_364);
        if (req->locationId.length() > 0)
            header += req->locationId;

        header.appendP(fb_esp_pgm_str_365);

        if (req->requestType == fb_esp_functions_request_type_list)
        {
            bool hasParam = false;

            if (req->pageSize > 0)
            {
                if (hasParam)
                    header.appendP(fb_esp_pgm_str_172);
                else
                    header.appendP(fb_esp_pgm_str_173);
                header.appendP(fb_esp_pgm_str_357);
                header.appendP(fb_esp_pgm_str_361);
                header += req->pageSize;
                hasParam = true;
            }

            if (req->pageToken.length() > 0)
            {
                if (hasParam)
                    header.appendP(fb_esp_pgm_str_172);
                else
                    header.appendP(fb_esp_pgm_str_173);
                header.appendP(fb_esp_pgm_str_358);
                header.appendP(fb_esp_pgm_str_361);
                header += req->pageToken;
                hasParam = true;
            }
        }

        if (req->requestType == fb_esp_functions_request_type_patch || req->requestType == fb_esp_functions_request_type_get_iam_policy || req->requestType == fb_esp_functions_request_type_gen_download_url || req->requestType == fb_esp_functions_request_type_delete || req->requestType == fb_esp_functions_request_type_get || req->requestType == fb_esp_functions_request_type_call || req->requestType == fb_esp_functions_request_type_set_iam_policy)
        {
            header.appendP(fb_esp_pgm_str_1);
            if (req->functionId.length() > 0)
                header += req->functionId;
            if (req->requestType == fb_esp_functions_request_type_call)
                header.appendP(fb_esp_pgm_str_366);
            else if (req->requestType == fb_esp_functions_request_type_set_iam_policy)
                header.appendP(fb_esp_pgm_str_401);
            else if (req->requestType == fb_esp_functions_request_type_gen_download_url)
                header.appendP(fb_esp_pgm_str_438);
            else if (req->requestType == fb_esp_functions_request_type_get_iam_policy)
            {
                header.appendP(fb_esp_pgm_str_465);
                if (req->policyVersion.length() > 0)
                {
                    header.appendP(fb_esp_pgm_str_173);
                    header.appendP(fb_esp_pgm_str_466);
                    header.appendP(fb_esp_pgm_str_361);
                    header += req->policyVersion;
                }
            }
            else if (req->requestType == fb_esp_functions_request_type_patch)
            {
                bool hasParam = false;

                if (req->updateMask->size() > 0)
                {
                    for (size_t i = 0; i < req->updateMask->size(); i++)
                    {
                        if (!hasParam)
                            header.appendP(fb_esp_pgm_str_173);
                        else
                            header.appendP(fb_esp_pgm_str_172);
                        header.appendP(fb_esp_pgm_str_470);
                        header += (*req->updateMask)[i];
                        hasParam = true;
                    }
                }
            }
        }
        else if (req->requestType == fb_esp_functions_request_type_gen_upload_url)
            header.appendP(fb_esp_pgm_str_439);
    }

    header.appendP(fb_esp_pgm_str_30);

    if (post)
    {
        if (req->payload.length() > 0)
        {
            header.appendP(fb_esp_pgm_str_8);
            header.appendP(fb_esp_pgm_str_129);
            header.appendP(fb_esp_pgm_str_21);
        }

        header.appendP(fb_esp_pgm_str_12);
        header += req->payload.length();
        header.appendP(fb_esp_pgm_str_21);
    }

    if (req->requestType == fb_esp_functions_request_type_upload || req->requestType == fb_esp_functions_request_type_pgm_upload)
    {
        header.appendP(fb_esp_pgm_str_8);
        header.appendP(fb_esp_pgm_str_447);
        header.appendP(fb_esp_pgm_str_21);

        size_t len = 0;
        if (req->requestType == fb_esp_functions_request_type_pgm_upload)
            len = req->pgmArcLen;
        else if (req->requestType == fb_esp_functions_request_type_upload)
            len = Signer.getCfg()->_int.fb_file.size();

        header.appendP(fb_esp_pgm_str_12);
        header += len;
        header.appendP(fb_esp_pgm_str_21);

        header.appendP(fb_esp_pgm_str_448);
        header.appendP(fb_esp_pgm_str_21);

        header.appendP(fb_esp_pgm_str_31);
        header += req->host;
        header.appendP(fb_esp_pgm_str_21);
    }
    else
    {
        header.appendP(fb_esp_pgm_str_31);

        if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources)
            header += req->host;
        else
        {
            header.appendP(fb_esp_pgm_str_363);
            header.appendP(fb_esp_pgm_str_120);
        }
        header.appendP(fb_esp_pgm_str_21);

        if (req->requestType != fb_esp_functions_request_type_upload_bucket_sources)
        {
            if (!Signer.getCfg()->signer.test_mode)
            {
                header.appendP(fb_esp_pgm_str_237);
                if (Signer.getTokenType() == token_type_oauth2_access_token)
                    header.appendP(fb_esp_pgm_str_271);

                header += Signer.getToken();
                header.appendP(fb_esp_pgm_str_21);
            }
        }
    }

    header.appendP(fb_esp_pgm_str_32);
    header.appendP(fb_esp_pgm_str_34);
    header.appendP(fb_esp_pgm_str_21);

    fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    int ret = fbdo->tcpSend(header.c_str());
    if (ret == 0 && req->payload.length() > 0)
        ret = fbdo->tcpSend(req->payload.c_str());

    header.clear();
    req->payload.clear();

    if (ret == 0)
    {
        fbdo->_ss.connected = true;
        if (req->requestType == fb_esp_functions_request_type_upload)
        {
            int available = Signer.getCfg()->_int.fb_file.available();
            int bufLen = 512;
            uint8_t *buf = new uint8_t[bufLen + 1];
            size_t read = 0;
            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                read = Signer.getCfg()->_int.fb_file.read(buf, available);
                if (fbdo->tcpClient.stream()->write(buf, read) != read)
                    break;
                available = Signer.getCfg()->_int.fb_file.available();
            }
            ut->delP(&buf);
            Signer.getCfg()->_int.fb_file.close();
        }
        else if (req->requestType == fb_esp_functions_request_type_pgm_upload)
        {
            int len = req->pgmArcLen;
            int available = len;
            int bufLen = 512;
            uint8_t *buf = new uint8_t[bufLen + 1];
            size_t pos = 0;
            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                memcpy_P(buf, req->pgmArc + pos, available);
                if (fbdo->tcpClient.stream()->write(buf, available) != (size_t)available)
                    break;
                pos += available;
                len -= available;
                available = len;
            }
            ut->delP(&buf);
        }

        if (handleResponse(fbdo))
        {
            fbdo->closeSession();
            Signer.getCfg()->_int.fb_processing = false;
            return true;
        }
    }
    else
        fbdo->_ss.connected = false;

    Signer.getCfg()->_int.fb_processing = false;
    return false;
}

void FB_Functions::rescon(FirebaseData *fbdo, const char *host)
{
    if (fbdo->_ss.cert_updated || !fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.con_mode != fb_esp_con_mode_functions || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
        fbdo->ethDNSWorkAround(&ut->config->spi_ethernet_module, host, 443);
    }
    fbdo->_ss.host = host;
    fbdo->_ss.con_mode = fb_esp_con_mode_functions;
}

bool FB_Functions::connect(FirebaseData *fbdo, const char *host)
{
    if (strlen(host) > 0)
    {
        rescon(fbdo, host);
        fbdo->tcpClient.begin(host, 443);
    }
    else
    {
        MBSTRING host;
        host.appendP(fb_esp_pgm_str_363);
        host.appendP(fb_esp_pgm_str_120);
        rescon(fbdo, host.c_str());
        fbdo->tcpClient.begin(host.c_str(), 443);
    }
    fbdo->_ss.max_payload_length = 0;
    return true;
}

bool FB_Functions::handleResponse(FirebaseData *fbdo)
{
    if (!fbdo->reconnect())
        return false;

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->tcpClient.stream();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    char *header = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int hBufPos = 0;
    int chunkBufSize = stream->available();
    int hstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int payloadRead = 0;
    size_t defaultChunkSize = fbdo->_ss.resp_size;
    struct fb_esp_auth_token_error_t error;
    error.code = -1;
    MBSTRING js;

    fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->_ss.content_length = -1;
    fbdo->_ss.payload_length = 0;
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    defaultChunkSize = 2048;
    bool envVarsBegin = false;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = stream->available();
        ut->idle();
    }

    chunkBufSize = stream->available();

    int availablePayload = chunkBufSize;

    dataTime = millis();

    fbdo->_ss.cfn.payload.clear();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0 || availablePayload > 0 || payloadRead < response.contentLen)
        {
            if (!fbdo->reconnect(dataTime))
                return false;

            chunkBufSize = stream->available();

            if (chunkBufSize <= 0 && availablePayload <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    //the first chunk can be http response header
                    header = (char *)ut->newP(chunkBufSize);
                    hstate = 1;
                    int readLen = ut->readLine(stream, header, chunkBufSize);
                    int pos = 0;

                    tmp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    ut->idle();
                    dataTime = millis();
                    if (tmp)
                    {
                        //http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(tmp);
                        fbdo->_ss.http_code = response.httpCode;
                        ut->delP(&tmp);
                    }
                }
                else
                {
                    ut->idle();
                    dataTime = millis();
                    //the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        //read one line of next header field until the empty header has found
                        tmp = (char *)ut->newP(chunkBufSize);
                        int readLen = ut->readLine(stream, tmp, chunkBufSize);
                        bool headerEnded = false;

                        //check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (tmp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (tmp[0] == '\r' && tmp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {
                            if (response.httpCode == 401)
                                Signer.authenticated = false;
                            else if (response.httpCode < 300)
                                Signer.authenticated = true;

                            //parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);
                            fbdo->_ss.chunked_encoding = response.isChunkedEnc;

                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
                                error.code = 0;

                            if (hstate == 1)
                                ut->delP(&header);
                            hstate = 0;

                            if (response.contentLen == 0)
                            {
                                ut->delP(&tmp);
                                break;
                            }
                        }
                        else
                        {
                            //accumulate the remaining header field
                            memcpy(header + hBufPos, tmp, readLen);
                            hBufPos += readLen;
                        }
                        ut->delP(&tmp);
                    }
                    else
                    {
                        //the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            pChunkIdx++;
                            pChunk = (char *)ut->newP(chunkBufSize + 1);

                            if (response.isChunkedEnc)
                                delay(10);
                            //read the avilable data
                            //chunk transfer encoding?
                            if (response.isChunkedEnc)
                                availablePayload = ut->readChunkedData(stream, pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                            else
                                availablePayload = ut->readLine(stream, pChunk, chunkBufSize);

                            if (availablePayload > 0)
                            {
                                fbdo->_ss.payload_length += availablePayload;
                                if (fbdo->_ss.max_payload_length < fbdo->_ss.payload_length)
                                    fbdo->_ss.max_payload_length = fbdo->_ss.payload_length;
                                payloadRead += availablePayload;
                                if (ut->strposP(pChunk, fb_esp_pgm_str_437, 0) == -1 && ut->strposP(pChunk, fb_esp_pgm_str_381, 0) == -1 && ut->strposP(pChunk, fb_esp_pgm_str_382, 0) == -1 && ut->strposP(pChunk, fb_esp_pgm_str_383, 0) == -1 && ut->strposP(pChunk, fb_esp_pgm_str_386, 0) == -1 && ut->strposP(pChunk, fb_esp_pgm_str_372, 0) == -1 && ut->strposP(pChunk, fb_esp_pgm_str_467, 0) == -1 && ut->strposP(pChunk, fb_esp_pgm_str_468, 0) == -1)
                                {
                                    if (ut->strposP(pChunk, fb_esp_pgm_str_374, 0) > -1)
                                        envVarsBegin = true;

                                    fbdo->_ss.cfn.payload += pChunk;

                                    if (envVarsBegin && ut->strposP(pChunk, fb_esp_pgm_str_469, 0) > -1)
                                        envVarsBegin = false;
                                }
                            }

                            ut->delP(&pChunk);

                            if (availablePayload < 0 || (payloadRead >= response.contentLen && !response.isChunkedEnc))
                            {
                                while (stream->available() > 0)
                                    stream->read();
                                break;
                            }
                        }
                        else
                        {
                            //read all the rest data
                            while (stream->available() > 0)
                                stream->read();
                            break;
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            ut->delP(&header);

        fbdo->_ss.cfn.payload.shrink_to_fit();

        //parse the payload
        if (fbdo->_ss.cfn.payload.length() > 0 && (fbdo->_ss.cfn.requestType != fb_esp_functions_request_type_upload && fbdo->_ss.cfn.requestType != fb_esp_functions_request_type_pgm_upload))
        {
            if (fbdo->_ss.cfn.payload[0] == '{')
            {
                if (!fbdo->_ss.jsonPtr)
                    fbdo->_ss.jsonPtr = new FirebaseJson();

                if (!fbdo->_ss.dataPtr)
                    fbdo->_ss.dataPtr = new FirebaseJsonData();

                fbdo->_ss.jsonPtr->setJsonData(fbdo->_ss.cfn.payload.c_str());
                fbdo->_ss.jsonPtr->get(*fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_257));

                if (fbdo->_ss.dataPtr->success)
                {
                    error.code = fbdo->_ss.dataPtr->intValue;
                    fbdo->_ss.jsonPtr->get(*fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_258));
    
                    if (fbdo->_ss.dataPtr->success)
                    {
                        fbdo->_ss.error = fbdo->_ss.dataPtr->to<const char *>();

                        fbdo->_ss.jsonPtr->get(*fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_418));
                        if (fbdo->_ss.dataPtr->success)
                        {
                            fbdo->_ss.error += ", ";
                            fbdo->_ss.error += fbdo->_ss.dataPtr->to<const char *>();
                        }

                        if (_deployTasks.size() > 0)
                        {
                            fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                            sendCallback(fbdo, fbdo->_ss.cfn.cbInfo.status, fbdo->_ss.error.c_str(), _deployTasks[_deployIndex].callback, _deployTasks[_deployIndex].statusInfo);
                        }
                    }
                }
                else
                {
                    error.code = 0;
                }

                fbdo->_ss.jsonPtr->clear();
                fbdo->_ss.dataPtr->clear();
            }
            fbdo->_ss.content_length = response.payloadLen;
        }
    }
    else
    {
        while (stream->available() > 0)
            stream->read();
    }

    if (fbdo->_ss.cfn.requestType == fb_esp_functions_request_type_upload || fbdo->_ss.cfn.requestType == fb_esp_functions_request_type_pgm_upload)
        return response.httpCode == 200;
    else
        return error.code == 0;
}

#if defined(ESP32)
void FB_Functions::runDeployTask(const char *taskName)
#elif defined(ESP8266)
void FB_Functions::runDeployTask()
#endif
{
#if defined(ESP32)

    static FB_Functions *_this = this;

    TaskFunction_t taskCode = [](void *param)
    {
        while (_this->_creation_task_enable)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);

            if (_this->_deployTasks.size() == 0)
                break;

            if (_this->_deployIndex < _this->_deployTasks.size() - 1)
                _this->_deployIndex++;
            else
                _this->_deployIndex = 0;

            struct fb_esp_deploy_task_info_t *taskInfo = &_this->_deployTasks[_this->_deployIndex];

            if (!taskInfo->done)
            {
                if (taskInfo->step == fb_esp_functions_creation_step_gen_upload_url)
                {
                    taskInfo->done = true;
                    _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_generate_upload_url, "", taskInfo->callback, taskInfo->statusInfo);
                    bool ret = _this->mGenerateUploadUrl(taskInfo->fbdo, taskInfo->config->_projectId.c_str(), taskInfo->config->_locationId.c_str());

                    if (ret)
                    {
                        if (!taskInfo->fbdo->_ss.jsonPtr)
                            taskInfo->fbdo->_ss.jsonPtr = new FirebaseJson();

                        if (!taskInfo->fbdo->_ss.arrPtr)
                            taskInfo->fbdo->_ss.arrPtr = new FirebaseJsonArray();

                        if (!taskInfo->fbdo->_ss.dataPtr)
                            taskInfo->fbdo->_ss.dataPtr = new FirebaseJsonData();

                        taskInfo->fbdo->_ss.jsonPtr->clear();
                        taskInfo->fbdo->_ss.jsonPtr->setJsonData(taskInfo->fbdo->_ss.cfn.payload.c_str());
        
                        taskInfo->fbdo->_ss.jsonPtr->get(*taskInfo->fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_440));
                
                        taskInfo->fbdo->_ss.jsonPtr->clear();
                        taskInfo->fbdo->_ss.arrPtr->clear();
                        MBSTRING().swap(taskInfo->fbdo->_ss.cfn.payload);
                        if (taskInfo->fbdo->_ss.dataPtr->success)
                        {
                            _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_deploy, taskInfo->callback, taskInfo->statusInfo);
                            _this->_deployTasks[_this->_deployIndex + 1].uploadUrl = taskInfo->fbdo->_ss.dataPtr->to<const char *>();
                        }
                    }
                    else
                        _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_upload_source_files)
                {
                    taskInfo->done = true;

                    _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_upload_source_file_in_progress, "", taskInfo->callback, taskInfo->statusInfo);
                    bool ret = _this->uploadSources(taskInfo->fbdo, taskInfo->config);

                    if (ret)
                    {
                        if (!taskInfo->fbdo->_ss.jsonPtr)
                            taskInfo->fbdo->_ss.jsonPtr = new FirebaseJson();

                        if (!taskInfo->fbdo->_ss.dataPtr)
                            taskInfo->fbdo->_ss.dataPtr = new FirebaseJsonData();

                        taskInfo->fbdo->_ss.jsonPtr->clear();
                        taskInfo->fbdo->_ss.jsonPtr->setJsonData(taskInfo->fbdo->_ss.cfn.payload.c_str());

                        taskInfo->fbdo->_ss.jsonPtr->get(*taskInfo->fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_457));
                       

                        if (taskInfo->fbdo->_ss.dataPtr->success)
                        {
                            const char *t = pgm2Str(fb_esp_pgm_str_383);
                            taskInfo->config->_funcCfg.add(t, taskInfo->fbdo->_ss.dataPtr->to<const char *>());
                            taskInfo->config->addUpdateMasks(t);
                           
                        }

                        taskInfo->fbdo->_ss.jsonPtr->clear();

                        _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                    }
                    else
                    {
                        if (taskInfo->fbdo->_ss.http_code == 302 || taskInfo->fbdo->_ss.http_code == 403)
                            taskInfo->fbdo->_ss.error.appendP(fb_esp_pgm_str_458);

                        _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                    }
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_upload_zip_file)
                {
                    taskInfo->done = true;
                    bool ret = false;
                    _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_upload_source_file_in_progress, "", taskInfo->callback, taskInfo->statusInfo);

                    if (taskInfo->config->_sourceType == functions_sources_type_local_archive)
                        ret = _this->uploadFile(taskInfo->fbdo, taskInfo->uploadUrl.c_str(), taskInfo->config->_uploadArchiveFile.c_str(), taskInfo->config->_uploadArchiveStorageType);
                    else if (taskInfo->config->_sourceType == functions_sources_type_flash_data)
                        ret = _this->uploadPGMArchive(taskInfo->fbdo, taskInfo->uploadUrl.c_str(), taskInfo->config->_pgmArc, taskInfo->config->_pgmArcLen);
                    taskInfo->uploadUrl.clear();
                    MBSTRING().swap(taskInfo->uploadUrl);
                    if (ret)
                    {
                        if (!taskInfo->fbdo->_ss.dataPtr)
                            taskInfo->fbdo->_ss.dataPtr = new FirebaseJsonData();

                        taskInfo->config->_funcCfg.set(pgm2Str(fb_esp_pgm_str_383), taskInfo->fbdo->_ss.dataPtr->to<const char *>());
                        taskInfo->config->addUpdateMasks(pgm2Str(fb_esp_pgm_str_383));
                      
                        _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                    }
                    else
                        _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_deploy)
                {
                    taskInfo->done = true;
                    bool ret = false;
                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                    _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);

                    ret = _this->deploy(taskInfo->fbdo, taskInfo->functionId.c_str(), taskInfo->config, taskInfo->patch);
                    taskInfo->fbdo->_ss.cfn.cbInfo.triggerUrl = taskInfo->config->_httpsTriggerUrl;
                    if (ret)
                        _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_set_iam_policy, taskInfo->callback, taskInfo->statusInfo);
                    else
                        _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_set_iam_policy)
                {
                    taskInfo->done = true;
                    bool ret = false;
                    static PolicyBuilder pol;
                    pol.json.setJsonData(taskInfo->policy.c_str());
                    ret = _this->mSetIamPolicy(taskInfo->fbdo, taskInfo->projectId.c_str(), taskInfo->locationId.c_str(), taskInfo->functionId.c_str(), &pol);
                    if (ret)
                    {
                        taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                        _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);
                    }
                    else
                        _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_delete)
                {
                    taskInfo->done = true;
                    bool ret = false;
                    MBSTRING t;
                    t.appendP(fb_esp_pgm_str_428);
                    t += taskInfo->projectId;
                    t.appendP(fb_esp_pgm_str_431);
                    ret = _this->mListOperations(taskInfo->fbdo, t.c_str(), "1", "");
                    if (ret)
                    {
                        taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                        if (!taskInfo->fbdo->_ss.dataPtr)
                            taskInfo->fbdo->_ss.dataPtr = new FirebaseJsonData();

                        if (!taskInfo->fbdo->_ss.jsonPtr)
                            taskInfo->fbdo->_ss.jsonPtr = new FirebaseJson();

                        taskInfo->fbdo->_ss.jsonPtr->clear();
                        taskInfo->fbdo->_ss.jsonPtr->setJsonData(taskInfo->fbdo->_ss.cfn.payload.c_str());
                        taskInfo->fbdo->_ss.jsonPtr->get(*taskInfo->fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_432));
                        _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, taskInfo->fbdo->_ss.dataPtr->to<const char *>(), taskInfo->callback, taskInfo->statusInfo);
                    }

                    ret = _this->mDeleteFunction(taskInfo->fbdo, taskInfo->projectId.c_str(), taskInfo->locationId.c_str(), taskInfo->functionId.c_str());
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_polling_status)
                {
                    if (millis() - _this->_lasPollMs > 5000 || _this->_lasPollMs == 0)
                    {
                        _this->_lasPollMs = millis();

                        if (!taskInfo->_delete && !taskInfo->active)
                        {

                            bool ret = _this->mGetFunction(taskInfo->fbdo, taskInfo->projectId.c_str(), taskInfo->locationId.c_str(), taskInfo->functionId.c_str());

                            if (ret)
                            {
                                if (_this->_function_status == fb_esp_functions_status_UNKNOWN || _this->_function_status == fb_esp_functions_status_OFFLINE)
                                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                                else if (_this->_function_status == fb_esp_functions_status_DEPLOY_IN_PROGRESS)
                                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                                else if (_this->_function_status == fb_esp_functions_status_DELETE_IN_PROGRESS)
                                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_delete_in_progress;
                                else if (_this->_function_status == fb_esp_functions_status_ACTIVE)
                                {
                                    if (taskInfo->setPolicy)
                                        taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_set_iam_policy_in_progress;
                                    else
                                        taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                                }

                                if (_this->_function_status != fb_esp_functions_status_UNKNOWN && _this->_function_status != fb_esp_functions_status_OFFLINE)
                                    _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, taskInfo->fbdo->_ss.error.c_str(), taskInfo->callback, taskInfo->statusInfo);
                            }

                            if (!ret || _this->_function_status == fb_esp_functions_status_ACTIVE || _this->_function_status == fb_esp_functions_status_UNKNOWN || _this->_function_status == fb_esp_functions_status_OFFLINE)
                            {
                                taskInfo->done = true;
                                taskInfo->_delete = true;
                                taskInfo->active = _this->_function_status == fb_esp_functions_status_ACTIVE;

                                if (_this->_function_status == fb_esp_functions_status_ACTIVE && taskInfo->setPolicy)
                                    _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_idle, taskInfo->callback, taskInfo->statusInfo);

                                if (_this->_function_status == fb_esp_functions_status_UNKNOWN || _this->_function_status == fb_esp_functions_status_OFFLINE)
                                    _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, fb_esp_functions_creation_step_delete, fb_esp_functions_creation_step_idle, taskInfo->callback, taskInfo->statusInfo);
                            }
                        }
                    }
                    yield();
                    vTaskDelay(5000 / portTICK_PERIOD_MS);
                }
            }

            size_t n = 0;
            for (size_t i = 0; i < _this->_deployTasks.size(); i++)
                if (_this->_deployTasks[i].done)
                    n++;

            if (n == _this->_deployTasks.size())
            {
                for (size_t i = 0; i < n; i++)
                {
                    struct fb_esp_deploy_task_info_t *taskInfo = &_this->_deployTasks[i];
                    MBSTRING().swap(taskInfo->uploadUrl);
                    MBSTRING().swap(taskInfo->projectId);
                    MBSTRING().swap(taskInfo->locationId);
                    MBSTRING().swap(taskInfo->functionId);
                    MBSTRING().swap(taskInfo->policy);
                    MBSTRING().swap(taskInfo->httpsTriggerUrl);
                    taskInfo->fbdo->_ss.long_running_task--;
                    taskInfo->fbdo->clear();
                    taskInfo->fbdo = nullptr;
                    taskInfo->callback = NULL;
                    taskInfo->config = nullptr;
                }
                _this->_deployTasks.clear();
                _this->_creation_task_enable = false;
            }

            yield();
        }

        Signer.getCfg()->_int.functions_check_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName, 12000, NULL, 3, &Signer.getCfg()->_int.functions_check_task_handle, 1);
#elif defined(ESP8266)

    if (_creation_task_enable)
    {
        delay(10);

        if (_deployTasks.size() == 0)
            return;

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
                sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_generate_upload_url, "", taskInfo->callback, taskInfo->statusInfo);
                bool ret = mGenerateUploadUrl(taskInfo->fbdo, taskInfo->config->_projectId.c_str(), taskInfo->config->_locationId.c_str());

                if (ret)
                {
                    if (!taskInfo->fbdo->_ss.dataPtr)
                        taskInfo->fbdo->_ss.dataPtr = new FirebaseJsonData();

                    if (!taskInfo->fbdo->_ss.jsonPtr)
                        taskInfo->fbdo->_ss.jsonPtr = new FirebaseJson();

                    if (!taskInfo->fbdo->_ss.arrPtr)
                        taskInfo->fbdo->_ss.arrPtr = new FirebaseJsonArray();

                    taskInfo->fbdo->_ss.jsonPtr->clear();
                    taskInfo->fbdo->_ss.jsonPtr->setJsonData(taskInfo->fbdo->_ss.cfn.payload.c_str());
               
                    taskInfo->fbdo->_ss.jsonPtr->get(*taskInfo->fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_440));
                
                    taskInfo->fbdo->_ss.jsonPtr->clear();
                    taskInfo->fbdo->_ss.arrPtr->clear();
                    MBSTRING().swap(taskInfo->fbdo->_ss.cfn.payload);
                    if (taskInfo->fbdo->_ss.dataPtr->success)
                    {
                        addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_deploy, taskInfo->callback, taskInfo->statusInfo);
                        _deployTasks[_deployIndex + 1].uploadUrl = taskInfo->fbdo->_ss.dataPtr->to<const char *>();
                    }
                }
                else
                    sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
            }

            if (taskInfo->step == fb_esp_functions_creation_step_upload_source_files)
            {
                taskInfo->done = true;

                sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_upload_source_file_in_progress, "", taskInfo->callback, taskInfo->statusInfo);
                bool ret = uploadSources(taskInfo->fbdo, taskInfo->config);

                if (ret)
                {
                    if (!taskInfo->fbdo->_ss.jsonPtr)
                        taskInfo->fbdo->_ss.jsonPtr = new FirebaseJson();

                    if (!taskInfo->fbdo->_ss.dataPtr)
                        taskInfo->fbdo->_ss.dataPtr = new FirebaseJsonData();

                    taskInfo->fbdo->_ss.jsonPtr->clear();
                    taskInfo->fbdo->_ss.jsonPtr->setJsonData(taskInfo->fbdo->_ss.cfn.payload.c_str());

        
                    taskInfo->fbdo->_ss.jsonPtr->get(*taskInfo->fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_457));
                

                    if (taskInfo->fbdo->_ss.dataPtr->success)
                    {
                       
                        taskInfo->config->_funcCfg.add(pgm2Str(fb_esp_pgm_str_383), taskInfo->fbdo->_ss.dataPtr->to<const char *>());
                        taskInfo->config->addUpdateMasks(pgm2Str(fb_esp_pgm_str_383));
                       
                    }

                    taskInfo->fbdo->_ss.jsonPtr->clear();

                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    if (taskInfo->fbdo->_ss.http_code == 302 || taskInfo->fbdo->_ss.http_code == 403)
                        taskInfo->fbdo->_ss.error.appendP(fb_esp_pgm_str_458);

                    sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == fb_esp_functions_creation_step_upload_zip_file)
            {
                taskInfo->done = true;
                bool ret = false;
                sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_upload_source_file_in_progress, "", taskInfo->callback, taskInfo->statusInfo);

                if (taskInfo->config->_sourceType == functions_sources_type_local_archive)
                    ret = uploadFile(taskInfo->fbdo, taskInfo->uploadUrl.c_str(), taskInfo->config->_uploadArchiveFile.c_str(), taskInfo->config->_uploadArchiveStorageType);
                else if (taskInfo->config->_sourceType == functions_sources_type_flash_data)
                    ret = uploadPGMArchive(taskInfo->fbdo, taskInfo->uploadUrl.c_str(), taskInfo->config->_pgmArc, taskInfo->config->_pgmArcLen);
                taskInfo->uploadUrl.clear();
                MBSTRING().swap(taskInfo->uploadUrl);
                if (ret)
                {
            
                    taskInfo->config->_funcCfg.set(pgm2Str(fb_esp_pgm_str_383), taskInfo->fbdo->_ss.dataPtr->to<const char *>());
                    taskInfo->config->addUpdateMasks(pgm2Str(fb_esp_pgm_str_383));
                   
                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                    sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
            }

            if (taskInfo->step == fb_esp_functions_creation_step_deploy)
            {
                taskInfo->done = true;
                bool ret = false;
                taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);

                ret = deploy(taskInfo->fbdo, taskInfo->functionId.c_str(), taskInfo->config, taskInfo->patch);
                taskInfo->fbdo->_ss.cfn.cbInfo.triggerUrl = taskInfo->config->_httpsTriggerUrl;
                if (ret)
                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_set_iam_policy, taskInfo->callback, taskInfo->statusInfo);
                else
                    sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
            }

            if (taskInfo->step == fb_esp_functions_creation_step_set_iam_policy)
            {
                taskInfo->done = true;
                bool ret = false;
                static PolicyBuilder pol;
                pol.json.setJsonData(taskInfo->policy.c_str());
                ret = mSetIamPolicy(taskInfo->fbdo, taskInfo->projectId.c_str(), taskInfo->locationId.c_str(), taskInfo->functionId.c_str(), &pol);
                if (ret)
                {
                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                    sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);
                }
                else
                    sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
            }

            if (taskInfo->step == fb_esp_functions_creation_step_delete)
            {
                taskInfo->done = true;
                bool ret = false;
                MBSTRING t;
                t.appendP(fb_esp_pgm_str_428);
                t += taskInfo->projectId;
                t.appendP(fb_esp_pgm_str_431);
                ret = mListOperations(taskInfo->fbdo, t.c_str(), "1", "");
                if (ret)
                {
                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                   
                    if (!taskInfo->fbdo->_ss.jsonPtr)
                        taskInfo->fbdo->_ss.jsonPtr = new FirebaseJson();

                    if (!taskInfo->fbdo->_ss.dataPtr)
                        taskInfo->fbdo->_ss.dataPtr = new FirebaseJsonData();

                    taskInfo->fbdo->_ss.jsonPtr->clear();
                    taskInfo->fbdo->_ss.jsonPtr->setJsonData(taskInfo->fbdo->_ss.cfn.payload.c_str());
                    taskInfo->fbdo->_ss.jsonPtr->get(*taskInfo->fbdo->_ss.dataPtr, pgm2Str(fb_esp_pgm_str_432));
                   
                    sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, taskInfo->fbdo->_ss.dataPtr->to<const char *>(), taskInfo->callback, taskInfo->statusInfo);
                }

                ret = mDeleteFunction(taskInfo->fbdo, taskInfo->projectId.c_str(), taskInfo->locationId.c_str(), taskInfo->functionId.c_str());
            }

            if (taskInfo->step == fb_esp_functions_creation_step_polling_status)
            {
                if (millis() - _lasPollMs > 5000 || _lasPollMs == 0)
                {
                    _lasPollMs = millis();

                    if (!taskInfo->_delete && !taskInfo->active)
                    {

                        bool ret = mGetFunction(taskInfo->fbdo, taskInfo->projectId.c_str(), taskInfo->locationId.c_str(), taskInfo->functionId.c_str());

                        if (ret)
                        {
                            if (_function_status == fb_esp_functions_status_UNKNOWN || _function_status == fb_esp_functions_status_OFFLINE)
                                taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                            else if (_function_status == fb_esp_functions_status_DEPLOY_IN_PROGRESS)
                                taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                            else if (_function_status == fb_esp_functions_status_DELETE_IN_PROGRESS)
                                taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_delete_in_progress;
                            else if (_function_status == fb_esp_functions_status_ACTIVE)
                            {
                                if (taskInfo->setPolicy)
                                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_set_iam_policy_in_progress;
                                else
                                    taskInfo->fbdo->_ss.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                            }

                            if (_function_status != fb_esp_functions_status_UNKNOWN && _function_status != fb_esp_functions_status_OFFLINE)
                                sendCallback(taskInfo->fbdo, taskInfo->fbdo->_ss.cfn.cbInfo.status, taskInfo->fbdo->_ss.error.c_str(), taskInfo->callback, taskInfo->statusInfo);
                        }

                        if (!ret || _function_status == fb_esp_functions_status_ACTIVE || _function_status == fb_esp_functions_status_UNKNOWN || _function_status == fb_esp_functions_status_OFFLINE)
                        {
                            taskInfo->done = true;
                            taskInfo->_delete = true;
                            taskInfo->active = _function_status == fb_esp_functions_status_ACTIVE;

                            if (_function_status == fb_esp_functions_status_ACTIVE && taskInfo->setPolicy)
                                addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_idle, taskInfo->callback, taskInfo->statusInfo);

                            if (_function_status == fb_esp_functions_status_UNKNOWN || _function_status == fb_esp_functions_status_OFFLINE)
                                addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, fb_esp_functions_creation_step_delete, fb_esp_functions_creation_step_idle, taskInfo->callback, taskInfo->statusInfo);
                        }
                    }
                }
            }
        }

        size_t n = 0;
        for (size_t i = 0; i < _deployTasks.size(); i++)
            if (_deployTasks[i].done)
                n++;

        if (_deployTasks.size() > 0 && n < _deployTasks.size())
            ut->set_scheduled_callback(std::bind(&FB_Functions::runDeployTask, this));

        if (n == _deployTasks.size())
        {
            for (size_t i = 0; i < n; i++)
            {
                struct fb_esp_deploy_task_info_t *taskInfo = &_deployTasks[i];
                MBSTRING().swap(taskInfo->uploadUrl);
                MBSTRING().swap(taskInfo->projectId);
                MBSTRING().swap(taskInfo->locationId);
                MBSTRING().swap(taskInfo->functionId);
                MBSTRING().swap(taskInfo->policy);
                MBSTRING().swap(taskInfo->httpsTriggerUrl);
                taskInfo->fbdo->_ss.long_running_task--;
                taskInfo->fbdo->clear();
                taskInfo->fbdo = nullptr;
                taskInfo->callback = NULL;
                taskInfo->config = nullptr;
            }

            _deployTasks.clear();
            _creation_task_enable = false;
        }
    }

#endif
}

#endif

#endif //ENABLE