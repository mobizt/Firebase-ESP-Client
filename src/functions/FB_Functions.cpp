/**
 * Google's Cloud Functions class, Functions.cpp version 1.1.17
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 1, 2022
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

bool FB_Functions::mCallFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId, MB_StringPtr data)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_call;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;

    req.payload = data;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();
    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_135), req.payload.c_str());
    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();
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
    fbdo->session.long_running_task++;

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

bool FB_Functions::createFunctionInt(FirebaseData *fbdo, MB_StringPtr functionId, FunctionsConfig *config, bool patch, FunctionsOperationCallback cb, FunctionsOperationStatusInfo *info)
{

    if (patch)
    {
        fbdo->session.cfn.cbInfo.functionId = functionId;
    }
    else
    {
        MB_String _functionId = functionId;
        if (_functionId.length() > 0)
            fbdo->session.cfn.cbInfo.functionId = functionId;
        else if (config->_entryPoint.length() > 0)
            fbdo->session.cfn.cbInfo.functionId = config->_entryPoint;
    }

    if (config->_sourceType == functions_sources_type_local_archive || config->_sourceType == functions_sources_type_flash_data)
    {
        if (config->_sourceType == functions_sources_type_local_archive)
        {
            if (config->_uploadArchiveStorageType == mem_storage_type_undefined)
                config->_uploadArchiveStorageType = mem_storage_type_flash;

            fbdo->session.cfn.filepath = config->_uploadArchiveFile;
            fbdo->session.cfn.storageType = config->_uploadArchiveStorageType;

            int sz = ut->mbfs->open(config->_uploadArchiveFile, mbfs_type config->_uploadArchiveStorageType, mb_fs_open_mode_read);

            if (sz < 0)
            {
                fbdo->session.response.code = MB_FS_ERROR_FILE_IO_ERROR;
                sendCallback(fbdo, fb_esp_functions_operation_status_error, fbdo->errorReason().c_str(), cb, info);
                return false;
            }

            fbdo->session.cfn.fileSize = sz;
        }

        // Close file and open later.
        // This is inefficient unless less memory usage than keep file opened
        // which causes the issue in ESP32 core 2.0.x
        ut->mbfs->close(mbfs_type config->_uploadArchiveStorageType);

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
        fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
        sendCallback(fbdo, fbdo->session.cfn.cbInfo.status, "", cb, info);
        MB_String _functionId = functionId;
        ret = deploy(fbdo, _functionId.c_str(), config, patch);
        fbdo->session.cfn.cbInfo.triggerUrl = config->_httpsTriggerUrl;

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
    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();

    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_387), config->_projectId.c_str());

    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_409), config->_locationId.c_str());

    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_453), pgm2Str(fb_esp_pgm_str_456));

    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_454), Signer.getToken());

    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_455), config->_bucketSourcesPath.c_str());

    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_upload_bucket_sources;

    req.payload = fbdo->session.jsonPtr->raw();

    fbdo->session.jsonPtr->clear();

    req.host += config->_locationId.c_str();
    req.host += fb_esp_pgm_str_397;
    req.host += config->_projectId.c_str();
    req.host += fb_esp_pgm_str_398;

    req.uri += fb_esp_pgm_str_1;
    req.uri += fb_esp_pgm_str_452;

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

    MB_String path;
    MB_String t;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();

    path = fb_esp_pgm_str_387;
    fbdo->session.jsonPtr->add(path.c_str(), config->_projectId.c_str());

    path = fb_esp_pgm_str_388;
    t += fb_esp_pgm_str_112;
    t += Signer.getCfg()->database_url.c_str();
    fbdo->session.jsonPtr->add(path.c_str(), t.c_str());

    if (config->_bucketId.length() > 0)
    {
        path = fb_esp_pgm_str_389;
        fbdo->session.jsonPtr->add(path.c_str(), config->_bucketId.c_str());
    }

    if (config->_locationId.length() > 0)
    {
        path = fb_esp_pgm_str_390;
        fbdo->session.jsonPtr->add(path.c_str(), config->_locationId.c_str());
    }

    t.clear();
    MB_String str = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();

    t = fb_esp_pgm_str_374;
    t += fb_esp_pgm_str_1;
    t += fb_esp_pgm_str_386;

    config->_funcCfg.set(t.c_str(), str.c_str());

    t = fb_esp_pgm_str_374;
    t += fb_esp_pgm_str_1;
    t += fb_esp_pgm_str_449;

    config->_funcCfg.set(t.c_str(), config->_projectId);

    config->_httpsTriggerUrl.clear();

    config->_httpsTriggerUrl.clear();

    str.clear();

    if (config->_triggerType == fb_esp_functions_trigger_type_https)
    {
        t.clear();
        t += fb_esp_pgm_str_112;
        t += config->_locationId.c_str();
        t += fb_esp_pgm_str_397;
        t += config->_projectId.c_str();
        t += fb_esp_pgm_str_398;
        t += fb_esp_pgm_str_1;
        t += config->_entryPoint;
        config->_funcCfg.set(pgm2Str(fb_esp_pgm_str_384), t.c_str());
        config->_httpsTriggerUrl = t;
        t.clear();
    }

    req.payload = config->_funcCfg.raw();
    config->_funcCfg.clear();

    return sendRequest(fbdo, &req);
}

void FB_Functions::sendCallback(FirebaseData *fbdo, fb_esp_functions_operation_status status, const char *message, FunctionsOperationCallback cb, FunctionsOperationStatusInfo *info)
{
    if (fbdo->session.cfn.last_status == status)
        return;

    fbdo->session.cfn.last_status = status;

    fbdo->session.cfn.cbInfo.status = status;
    fbdo->session.cfn.cbInfo.errorMsg = message;

    if (cb)
        cb(fbdo->session.cfn.cbInfo);

    if (info)
    {
        info->errorMsg = fbdo->session.cfn.cbInfo.errorMsg;
        info->status = fbdo->session.cfn.cbInfo.status;
    }
}

bool FB_Functions::mSetIamPolicy(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId, PolicyBuilder *policy, MB_StringPtr updateMask)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_set_iam_policy;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();

    if (policy)
    {
        static FirebaseJson js;
        js.clear();
        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_399), policy->json);
    }

    MB_String _updateMask = updateMask;

    if (_updateMask.length() > 0)
    {
        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_400), _updateMask);
    }

    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetIamPolicy(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId, MB_StringPtr version)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_get_iam_policy;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;
    req.policyVersion = version;
    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId)
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
        if (!fbdo->session.jsonPtr)
            fbdo->session.jsonPtr = new FirebaseJson();

        if (!fbdo->session.dataPtr)
            fbdo->session.dataPtr = new FirebaseJsonData();

        fbdo->session.jsonPtr->clear();
        fbdo->session.jsonPtr->setJsonData(fbdo->session.cfn.payload.c_str());
        fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_419));

        if (fbdo->session.dataPtr->success)
        {
            MB_String s = fb_esp_pgm_str_420;
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;

            s = fb_esp_pgm_str_421;
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_ACTIVE;

            s = fb_esp_pgm_str_422;
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_OFFLINE;

            s = fb_esp_pgm_str_423;
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_DEPLOY_IN_PROGRESS;

            s = fb_esp_pgm_str_424;
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_DELETE_IN_PROGRESS;

            s = fb_esp_pgm_str_425;
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), s.c_str()) == 0)
                _function_status = fb_esp_functions_status_UNKNOWN;
            s.clear();
        }
    }

    return ret;
}

bool FB_Functions::mListFunctions(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_list;
    req.projectId = projectId;
    req.locationId = locationId;
    MB_String _pageSize = pageSize;
    req.pageSize = atoi(_pageSize.c_str());
    req.pageToken = pageToken;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mListOperations(FirebaseData *fbdo, MB_StringPtr filter, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_list_operations;
    req.filter = filter;
    MB_String _pageSize = pageSize;
    req.pageSize = atoi(_pageSize.c_str());
    req.pageToken = pageToken;

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mDeleteFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_delete;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateDownloadUrl(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId, MB_StringPtr versionId)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_gen_download_url;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;
    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();

    MB_String _versionId = versionId;

    if (_versionId.length() > 0)
    {
        req.versionId = atoi(_versionId.c_str());
        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_437), _versionId);
    }

    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();

    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateUploadUrl(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId)
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
            req.filePath += fb_esp_pgm_str_1;
    }

    req.filePath += filePath;
    req.storageType = storageType;

    struct fb_esp_url_info_t info;
    ut->getUrlInfo(uploadUrl, info);
    req.host = info.host;
    req.uri += fb_esp_pgm_str_1;
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
    req.uri += fb_esp_pgm_str_1;
    req.uri += info.uri;
    return sendRequest(fbdo, &req);
}

void FB_Functions::begin(UtilsClass *u)
{
    ut = u;
}

bool FB_Functions::sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req)
{
    if (fbdo->tcpClient.reserved)
        return false;

    fbdo->session.http_code = 0;

    if (!Signer.getCfg())
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

#ifdef ENABLE_RTDB
    if (fbdo->session.rtdb.pause)
        return true;
#endif

    if (!fbdo->reconnect())
        return false;

    if (!Signer.tokenReady())
        return false;

    if (Signer.getCfg()->internal.fb_processing)
        return false;

    Signer.getCfg()->internal.fb_processing = true;

    fbdo->clear();

    connect(fbdo, req->host.c_str());

    return functions_sendRequest(fbdo, req);
}

bool FB_Functions::functions_sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req)
{
    bool post = false;
    fbdo->session.cfn.requestType = req->requestType;

    MB_String header;
    if (req->requestType == fb_esp_functions_request_type_get_iam_policy || req->requestType == fb_esp_functions_request_type_list_operations || req->requestType == fb_esp_functions_request_type_get || req->requestType == fb_esp_functions_request_type_get_iam_policy || req->requestType == fb_esp_functions_request_type_list)
        header += fb_esp_pgm_str_25;
    else if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources || req->requestType == fb_esp_functions_request_type_call || req->requestType == fb_esp_functions_request_type_create || req->requestType == fb_esp_functions_request_type_gen_download_url || req->requestType == fb_esp_functions_request_type_gen_upload_url || req->requestType == fb_esp_functions_request_type_set_iam_policy || req->requestType == fb_esp_functions_request_type_test_iam_policy)
    {
        header += fb_esp_pgm_str_24;
        post = true;
    }
    else if (req->requestType == fb_esp_functions_request_type_patch)
    {
        header += fb_esp_pgm_str_26;
        post = true;
    }
    else if (req->requestType == fb_esp_functions_request_type_delete)
        header += fb_esp_pgm_str_27;
    else if (req->requestType == fb_esp_functions_request_type_upload || req->requestType == fb_esp_functions_request_type_pgm_upload)
        header += fb_esp_pgm_str_23;

    header += fb_esp_pgm_str_6;

    if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources || req->requestType == fb_esp_functions_request_type_upload || req->requestType == fb_esp_functions_request_type_pgm_upload)
        header += req->uri;
    else if (req->requestType == fb_esp_functions_request_type_list_operations)
    {
        header += fb_esp_pgm_str_426;
        bool hasParam = false;

        if (req->filter.length() > 0)
        {
            hasParam = true;
            header += fb_esp_pgm_str_173;
            header += fb_esp_pgm_str_427;
            header += req->filter;
        }

        if (req->pageSize > 0)
        {
            if (hasParam)
                header += fb_esp_pgm_str_172;
            else
                header += fb_esp_pgm_str_173;
            header += fb_esp_pgm_str_357;
            header += fb_esp_pgm_str_361;
            header += req->pageSize;
            hasParam = true;
        }

        if (req->pageToken.length() > 0)
        {
            if (hasParam)
                header += fb_esp_pgm_str_172;
            else
                header += fb_esp_pgm_str_173;
            header += fb_esp_pgm_str_358;
            header += fb_esp_pgm_str_361;
            header += req->pageToken;
            hasParam = true;
        }
    }
    else
    {
        header += fb_esp_pgm_str_326;

        if (req->projectId.length() == 0)
            header += Signer.getCfg()->service_account.data.project_id;
        else
            header += req->projectId;

        header += fb_esp_pgm_str_364;
        if (req->locationId.length() > 0)
            header += req->locationId;

        header += fb_esp_pgm_str_365;

        if (req->requestType == fb_esp_functions_request_type_list)
        {
            bool hasParam = false;

            if (req->pageSize > 0)
            {
                if (hasParam)
                    header += fb_esp_pgm_str_172;
                else
                    header += fb_esp_pgm_str_173;
                header += fb_esp_pgm_str_357;
                header += fb_esp_pgm_str_361;
                header += req->pageSize;
                hasParam = true;
            }

            if (req->pageToken.length() > 0)
            {
                if (hasParam)
                    header += fb_esp_pgm_str_172;
                else
                    header += fb_esp_pgm_str_173;
                header += fb_esp_pgm_str_358;
                header += fb_esp_pgm_str_361;
                header += req->pageToken;
                hasParam = true;
            }
        }

        if (req->requestType == fb_esp_functions_request_type_patch || req->requestType == fb_esp_functions_request_type_get_iam_policy || req->requestType == fb_esp_functions_request_type_gen_download_url || req->requestType == fb_esp_functions_request_type_delete || req->requestType == fb_esp_functions_request_type_get || req->requestType == fb_esp_functions_request_type_call || req->requestType == fb_esp_functions_request_type_set_iam_policy)
        {
            header += fb_esp_pgm_str_1;
            if (req->functionId.length() > 0)
                header += req->functionId;
            if (req->requestType == fb_esp_functions_request_type_call)
                header += fb_esp_pgm_str_366;
            else if (req->requestType == fb_esp_functions_request_type_set_iam_policy)
                header += fb_esp_pgm_str_401;
            else if (req->requestType == fb_esp_functions_request_type_gen_download_url)
                header += fb_esp_pgm_str_438;
            else if (req->requestType == fb_esp_functions_request_type_get_iam_policy)
            {
                header += fb_esp_pgm_str_465;
                if (req->policyVersion.length() > 0)
                {
                    header += fb_esp_pgm_str_173;
                    header += fb_esp_pgm_str_466;
                    header += fb_esp_pgm_str_361;
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
                            header += fb_esp_pgm_str_173;
                        else
                            header += fb_esp_pgm_str_172;
                        header += fb_esp_pgm_str_470;
                        header += (*req->updateMask)[i];
                        hasParam = true;
                    }
                }
            }
        }
        else if (req->requestType == fb_esp_functions_request_type_gen_upload_url)
            header += fb_esp_pgm_str_439;
    }

    header += fb_esp_pgm_str_30;

    if (post)
    {
        if (req->payload.length() > 0)
        {
            header += fb_esp_pgm_str_8;
            header += fb_esp_pgm_str_129;
            header += fb_esp_pgm_str_21;
        }

        header += fb_esp_pgm_str_12;
        header += req->payload.length();
        header += fb_esp_pgm_str_21;
    }

    if (req->requestType == fb_esp_functions_request_type_upload || req->requestType == fb_esp_functions_request_type_pgm_upload)
    {
        header += fb_esp_pgm_str_8;
        header += fb_esp_pgm_str_447;
        header += fb_esp_pgm_str_21;

        size_t len = 0;
        if (req->requestType == fb_esp_functions_request_type_pgm_upload)
            len = req->pgmArcLen;
        else if (req->requestType == fb_esp_functions_request_type_upload)
            len = fbdo->session.cfn.fileSize;

        header += fb_esp_pgm_str_12;
        header += len;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_448;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_31;
        header += req->host;
        header += fb_esp_pgm_str_21;
    }
    else
    {
        header += fb_esp_pgm_str_31;

        if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources)
            header += req->host;
        else
        {
            header += fb_esp_pgm_str_363;
            header += fb_esp_pgm_str_120;
        }
        header += fb_esp_pgm_str_21;

        if (req->requestType != fb_esp_functions_request_type_upload_bucket_sources)
        {
            if (!Signer.getCfg()->signer.test_mode)
            {
                header += fb_esp_pgm_str_237;
                if (Signer.getTokenType() == token_type_oauth2_access_token)
                    header += fb_esp_pgm_str_209;

                header += Signer.getToken();
                header += fb_esp_pgm_str_21;
            }
        }
    }

    header += fb_esp_pgm_str_32;
    header += fb_esp_pgm_str_34;

    ut->getCustomHeaders(header);

    header += fb_esp_pgm_str_21;

    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    fbdo->tcpClient.send(header.c_str());
    if (fbdo->session.response.code > 0 && req->payload.length() > 0)
        fbdo->tcpClient.send(req->payload.c_str());

    header.clear();
    req->payload.clear();

    if (fbdo->session.response.code > 0)
    {

        fbdo->session.connected = true;
        if (req->requestType == fb_esp_functions_request_type_upload)
        {
            // This is inefficient unless less memory usage than keep file opened
            // which causes the issue in ESP32 core 2.0.x

            ut->mbfs->open(fbdo->session.cfn.filepath, mbfs_type fbdo->session.cfn.storageType, mb_fs_open_mode_read);

            fbdo->session.cfn.filepath.clear();

            int available = ut->mbfs->available(mbfs_type fbdo->session.cfn.storageType);

            int bufLen = Signer.getCfg()->functions.upload_buffer_size;
            if (bufLen < 512)
                bufLen = 512;

            if (bufLen > 1024 * 16)
                bufLen = 1024 * 16;

            uint8_t *buf = (uint8_t *)ut->newP(bufLen + 1, false);
            int read = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;

                read = ut->mbfs->read(mbfs_type fbdo->session.cfn.storageType, buf, available);

                if (fbdo->tcpClient.write(buf, read) != read)
                    break;

                available = ut->mbfs->available(mbfs_type fbdo->session.cfn.storageType);
            }

            ut->delP(&buf);

            ut->mbfs->close(mbfs_type fbdo->session.cfn.storageType);
        }
        else if (req->requestType == fb_esp_functions_request_type_pgm_upload)
        {
            int len = req->pgmArcLen;
            int available = len;

            int bufLen = Signer.getCfg()->functions.upload_buffer_size;
            if (bufLen < 512)
                bufLen = 512;

            if (bufLen > 1024 * 16)
                bufLen = 1024 * 16;

            uint8_t *buf = (uint8_t *)ut->newP(bufLen + 1, false);
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

            ut->delP(&buf);
        }

        if (handleResponse(fbdo))
        {
            fbdo->closeSession();
            Signer.getCfg()->internal.fb_processing = false;
            return true;
        }
    }
    else
        fbdo->session.connected = false;

    Signer.getCfg()->internal.fb_processing = false;
    return false;
}

void FB_Functions::rescon(FirebaseData *fbdo, const char *host)
{
    if (fbdo->session.cert_updated || !fbdo->session.connected || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout || fbdo->session.con_mode != fb_esp_con_mode_functions || strcmp(host, fbdo->session.host.c_str()) != 0)
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
        host += fb_esp_pgm_str_363;
        host += fb_esp_pgm_str_120;
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

    unsigned long dataTime = millis();

    char *pChunk = nullptr;
    char *temp = nullptr;
    char *header = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int hBufPos = 0;
    int chunkBufSize = fbdo->tcpClient.available();
    int hstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int payloadRead = 0;
    size_t defaultChunkSize = fbdo->session.resp_size;
    struct fb_esp_auth_token_error_t error;
    error.code = -1;
    MB_String js;

    fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->session.content_length = -1;
    fbdo->session.payload_length = 0;
    fbdo->session.chunked_encoding = false;
    fbdo->session.buffer_ovf = false;

    defaultChunkSize = 2048;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = fbdo->tcpClient.available();
        ut->idle();
    }

    chunkBufSize = fbdo->tcpClient.available();

    int availablePayload = chunkBufSize;

    dataTime = millis();

    fbdo->session.cfn.payload.clear();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0 || availablePayload > 0 || payloadRead < response.contentLen)
        {
            if (!fbdo->reconnect(dataTime))
                return false;

            chunkBufSize = fbdo->tcpClient.available();

            if (chunkBufSize <= 0 && availablePayload <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    // the first chunk can be http response header
                    header = (char *)ut->newP(chunkBufSize);
                    hstate = 1;
                    int readLen = fbdo->tcpClient.readLine(header, chunkBufSize);
                    int pos = 0;

                    temp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    ut->idle();
                    dataTime = millis();
                    if (temp)
                    {
                        // http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(temp);
                        fbdo->session.response.code = response.httpCode;
                        ut->delP(&temp);
                    }
                }
                else
                {
                    ut->idle();
                    dataTime = millis();
                    // the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        // read one line of next header field until the empty header has found
                        temp = (char *)ut->newP(chunkBufSize);
                        int readLen = fbdo->tcpClient.readLine(temp, chunkBufSize);
                        bool headerEnded = false;

                        // check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (temp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (temp[0] == '\r' && temp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {
                            if (response.httpCode == 401)
                                Signer.authenticated = false;
                            else if (response.httpCode < 300)
                                Signer.authenticated = true;

                            // parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            fbdo->session.http_code = response.httpCode;

                            fbdo->session.chunked_encoding = response.isChunkedEnc;

                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
                                error.code = 0;

                            if (hstate == 1)
                                ut->delP(&header);
                            hstate = 0;

                            if (response.contentLen == 0)
                            {
                                ut->delP(&temp);
                                break;
                            }
                        }
                        else
                        {
                            // accumulate the remaining header field
                            memcpy(header + hBufPos, temp, readLen);
                            hBufPos += readLen;
                        }
                        ut->delP(&temp);
                    }
                    else
                    {
                        // the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            pChunkIdx++;
                            pChunk = (char *)ut->newP(chunkBufSize + 1);

                            if (response.isChunkedEnc)
                                delay(10);
                            // read the avilable data
                            // chunk transfer encoding?
                            if (response.isChunkedEnc)
                                availablePayload = fbdo->tcpClient.readChunkedData(pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                            else
                                availablePayload = fbdo->tcpClient.readLine(pChunk, chunkBufSize);

                            if (availablePayload > 0)
                            {
                                fbdo->session.payload_length += availablePayload;
                                if (fbdo->session.max_payload_length < fbdo->session.payload_length)
                                    fbdo->session.max_payload_length = fbdo->session.payload_length;
                                payloadRead += availablePayload;

                                fbdo->session.cfn.payload += pChunk;
                            }

                            ut->delP(&pChunk);

                            if (availablePayload < 0 || (payloadRead >= response.contentLen && !response.isChunkedEnc))
                            {
                                fbdo->tcpClient.flush();
                                break;
                            }
                        }
                        else
                        {
                            // read all the rest data
                            fbdo->tcpClient.flush();
                            break;
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            ut->delP(&header);

        // parse the payload
        if (fbdo->session.cfn.payload.length() > 0 && (fbdo->session.cfn.requestType != fb_esp_functions_request_type_upload && fbdo->session.cfn.requestType != fb_esp_functions_request_type_pgm_upload))
        {
            if (fbdo->session.cfn.payload[0] == '{')
            {
                if (!fbdo->session.jsonPtr)
                    fbdo->session.jsonPtr = new FirebaseJson();

                if (!fbdo->session.dataPtr)
                    fbdo->session.dataPtr = new FirebaseJsonData();

                int errType = 0;

                fbdo->session.jsonPtr->setJsonData(fbdo->session.cfn.payload.c_str());
                fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_257));

                if (fbdo->session.dataPtr->success)
                    errType = 1;
                else
                {
                    fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_84));
                    if (fbdo->session.dataPtr->success)
                        errType = 2;
                }

                if (fbdo->session.dataPtr->success)
                {
                    error.code = fbdo->session.dataPtr->intValue;

                    if (errType == 1)
                        fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_258));
                    else if (errType == 2)
                        fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_432));

                    if (fbdo->session.dataPtr->success)
                    {
                        fbdo->session.error = fbdo->session.dataPtr->to<const char *>();

                        fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_418));
                        if (fbdo->session.dataPtr->success)
                        {
                            fbdo->session.error = fbdo->session.dataPtr->to<const char *>();
                        }

                        if (_deployTasks.size() > 0)
                        {
                            fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                            sendCallback(fbdo, fbdo->session.cfn.cbInfo.status, fbdo->session.error.c_str(), _deployTasks[_deployIndex].callback, _deployTasks[_deployIndex].statusInfo);
                        }
                    }
                }
                else
                {
                    error.code = 0;
                }

                fbdo->session.jsonPtr->clear();
                fbdo->session.dataPtr->clear();
            }
            fbdo->session.content_length = response.payloadLen;
        }
    }
    else
    {
        fbdo->tcpClient.flush();
    }

    if (fbdo->session.cfn.requestType == fb_esp_functions_request_type_upload || fbdo->session.cfn.requestType == fb_esp_functions_request_type_pgm_upload)
        return response.httpCode == 200;
    else
        return error.code == 0;
}

#if defined(ESP32)
void FB_Functions::runDeployTask(const char *taskName)
#elif defined(ESP8266) || defined(FB_ENABLE_EXTERNAL_CLIENT)
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
                    bool ret = _this->mGenerateUploadUrl(taskInfo->fbdo, toStringPtr(taskInfo->config->_projectId), toStringPtr(taskInfo->config->_locationId));

                    if (ret)
                    {
                        if (!taskInfo->fbdo->session.jsonPtr)
                            taskInfo->fbdo->session.jsonPtr = new FirebaseJson();

                        if (!taskInfo->fbdo->session.arrPtr)
                            taskInfo->fbdo->session.arrPtr = new FirebaseJsonArray();

                        if (!taskInfo->fbdo->session.dataPtr)
                            taskInfo->fbdo->session.dataPtr = new FirebaseJsonData();

                        taskInfo->fbdo->session.jsonPtr->clear();
                        taskInfo->fbdo->session.jsonPtr->setJsonData(taskInfo->fbdo->session.cfn.payload.c_str());

                        taskInfo->fbdo->session.jsonPtr->get(*taskInfo->fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_440));

                        taskInfo->fbdo->session.jsonPtr->clear();
                        taskInfo->fbdo->session.arrPtr->clear();
                        taskInfo->fbdo->session.cfn.payload.clear();
                        if (taskInfo->fbdo->session.dataPtr->success)
                        {
                            _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_deploy, taskInfo->callback, taskInfo->statusInfo);
                            _this->_deployTasks[_this->_deployIndex + 1].uploadUrl = taskInfo->fbdo->session.dataPtr->to<const char *>();
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
                        if (!taskInfo->fbdo->session.jsonPtr)
                            taskInfo->fbdo->session.jsonPtr = new FirebaseJson();

                        if (!taskInfo->fbdo->session.dataPtr)
                            taskInfo->fbdo->session.dataPtr = new FirebaseJsonData();

                        taskInfo->fbdo->session.jsonPtr->clear();
                        taskInfo->fbdo->session.jsonPtr->setJsonData(taskInfo->fbdo->session.cfn.payload.c_str());

                        taskInfo->fbdo->session.jsonPtr->get(*taskInfo->fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_457));

                        if (taskInfo->fbdo->session.dataPtr->success)
                        {
                            const char *t = pgm2Str(fb_esp_pgm_str_383);
                            taskInfo->config->_funcCfg.add(t, taskInfo->fbdo->session.dataPtr->to<const char *>());
                            taskInfo->config->addUpdateMasks(t);
                        }

                        taskInfo->fbdo->session.jsonPtr->clear();

                        _this->addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                    }
                    else
                    {
                        if (taskInfo->fbdo->session.response.code == 302 || taskInfo->fbdo->session.response.code == 403)
                            taskInfo->fbdo->session.error += fb_esp_pgm_str_458;

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

                    if (ret)
                    {
                        if (!taskInfo->fbdo->session.dataPtr)
                            taskInfo->fbdo->session.dataPtr = new FirebaseJsonData();

                        taskInfo->config->_funcCfg.set(pgm2Str(fb_esp_pgm_str_383), taskInfo->uploadUrl.c_str());
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
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                    _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);

                    ret = _this->deploy(taskInfo->fbdo, taskInfo->functionId.c_str(), taskInfo->config, taskInfo->patch);
                    taskInfo->fbdo->session.cfn.cbInfo.triggerUrl = taskInfo->config->_httpsTriggerUrl;
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
                    ret = _this->mSetIamPolicy(taskInfo->fbdo, toStringPtr(taskInfo->projectId), toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId), &pol, toStringPtr(_EMPTY_STR));
                    if (ret)
                    {
                        taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                        _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);
                    }
                    else
                        _this->sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_delete)
                {
                    taskInfo->done = true;
                    bool ret = false;
                    MB_String t;
                    t += fb_esp_pgm_str_428;
                    t += taskInfo->projectId;
                    t += fb_esp_pgm_str_431;
                    ret = _this->mListOperations(taskInfo->fbdo, toStringPtr(t.c_str()), toStringPtr("1"), toStringPtr(_EMPTY_STR));
                    if (ret)
                    {
                        taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                        if (!taskInfo->fbdo->session.dataPtr)
                            taskInfo->fbdo->session.dataPtr = new FirebaseJsonData();

                        if (!taskInfo->fbdo->session.jsonPtr)
                            taskInfo->fbdo->session.jsonPtr = new FirebaseJson();

                        taskInfo->fbdo->session.jsonPtr->clear();
                        taskInfo->fbdo->session.jsonPtr->setJsonData(taskInfo->fbdo->session.cfn.payload.c_str());
                        taskInfo->fbdo->session.jsonPtr->get(*taskInfo->fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_432));
                        _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, taskInfo->fbdo->session.dataPtr->to<const char *>(), taskInfo->callback, taskInfo->statusInfo);
                    }

                    ret = _this->mDeleteFunction(taskInfo->fbdo, toStringPtr(taskInfo->projectId), toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId));
                }
                else if (taskInfo->step == fb_esp_functions_creation_step_polling_status)
                {
                    if (millis() - _this->_lasPollMs > 5000 || _this->_lasPollMs == 0)
                    {
                        _this->_lasPollMs = millis();

                        if (!taskInfo->_delete && !taskInfo->active)
                        {

                            bool ret = _this->mGetFunction(taskInfo->fbdo, toStringPtr(taskInfo->projectId), toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId));

                            if (ret)
                            {
                                if (_this->_function_status == fb_esp_functions_status_UNKNOWN || _this->_function_status == fb_esp_functions_status_OFFLINE)
                                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                                else if (_this->_function_status == fb_esp_functions_status_DEPLOY_IN_PROGRESS)
                                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                                else if (_this->_function_status == fb_esp_functions_status_DELETE_IN_PROGRESS)
                                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_delete_in_progress;
                                else if (_this->_function_status == fb_esp_functions_status_ACTIVE)
                                {
                                    if (taskInfo->setPolicy)
                                        taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_set_iam_policy_in_progress;
                                    else
                                        taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                                }

                                if (_this->_function_status != fb_esp_functions_status_UNKNOWN && _this->_function_status != fb_esp_functions_status_OFFLINE)
                                    _this->sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, taskInfo->fbdo->session.error.c_str(), taskInfo->callback, taskInfo->statusInfo);
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
                _this->_deployTasks.clear();
                _this->_creation_task_enable = false;
            }

            yield();
        }

        Signer.getCfg()->internal.functions_check_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName, 12000, NULL, 3, &Signer.getCfg()->internal.functions_check_task_handle, 1);
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
                bool ret = mGenerateUploadUrl(taskInfo->fbdo, toStringPtr(taskInfo->config->_projectId), toStringPtr(taskInfo->config->_locationId));

                if (ret)
                {
                    if (!taskInfo->fbdo->session.dataPtr)
                        taskInfo->fbdo->session.dataPtr = new FirebaseJsonData();

                    if (!taskInfo->fbdo->session.jsonPtr)
                        taskInfo->fbdo->session.jsonPtr = new FirebaseJson();

                    if (!taskInfo->fbdo->session.arrPtr)
                        taskInfo->fbdo->session.arrPtr = new FirebaseJsonArray();

                    taskInfo->fbdo->session.jsonPtr->clear();
                    taskInfo->fbdo->session.jsonPtr->setJsonData(taskInfo->fbdo->session.cfn.payload.c_str());

                    taskInfo->fbdo->session.jsonPtr->get(*taskInfo->fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_440));

                    taskInfo->fbdo->session.jsonPtr->clear();
                    taskInfo->fbdo->session.arrPtr->clear();
                    taskInfo->fbdo->session.cfn.payload.clear();
                    if (taskInfo->fbdo->session.dataPtr->success)
                    {
                        addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_deploy, taskInfo->callback, taskInfo->statusInfo);
                        _deployTasks[_deployIndex + 1].uploadUrl = taskInfo->fbdo->session.dataPtr->to<const char *>();
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
                    if (!taskInfo->fbdo->session.jsonPtr)
                        taskInfo->fbdo->session.jsonPtr = new FirebaseJson();

                    if (!taskInfo->fbdo->session.dataPtr)
                        taskInfo->fbdo->session.dataPtr = new FirebaseJsonData();

                    taskInfo->fbdo->session.jsonPtr->clear();
                    taskInfo->fbdo->session.jsonPtr->setJsonData(taskInfo->fbdo->session.cfn.payload.c_str());

                    taskInfo->fbdo->session.jsonPtr->get(*taskInfo->fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_457));

                    if (taskInfo->fbdo->session.dataPtr->success)
                    {

                        taskInfo->config->_funcCfg.add(pgm2Str(fb_esp_pgm_str_383), taskInfo->fbdo->session.dataPtr->to<const char *>());
                        taskInfo->config->addUpdateMasks(pgm2Str(fb_esp_pgm_str_383));
                    }

                    taskInfo->fbdo->session.jsonPtr->clear();

                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    if (taskInfo->fbdo->session.response.code == 302 || taskInfo->fbdo->session.response.code == 403)
                        taskInfo->fbdo->session.error += fb_esp_pgm_str_458;

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

                if (ret)
                {

                    taskInfo->config->_funcCfg.set(pgm2Str(fb_esp_pgm_str_383), taskInfo->uploadUrl.c_str());
                    taskInfo->config->addUpdateMasks(pgm2Str(fb_esp_pgm_str_383));

                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep, fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                    sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
                taskInfo->uploadUrl.clear();
            }

            if (taskInfo->step == fb_esp_functions_creation_step_deploy)
            {
                taskInfo->done = true;
                bool ret = false;
                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);

                ret = deploy(taskInfo->fbdo, taskInfo->functionId.c_str(), taskInfo->config, taskInfo->patch);
                taskInfo->fbdo->session.cfn.cbInfo.triggerUrl = taskInfo->config->_httpsTriggerUrl;
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
                ret = mSetIamPolicy(taskInfo->fbdo, toStringPtr(taskInfo->projectId), toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId), &pol, toStringPtr(_EMPTY_STR));
                if (ret)
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                    sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, "", taskInfo->callback, taskInfo->statusInfo);
                }
                else
                    sendCallback(taskInfo->fbdo, fb_esp_functions_operation_status_error, taskInfo->fbdo->errorReason().c_str(), taskInfo->callback, taskInfo->statusInfo);
            }

            if (taskInfo->step == fb_esp_functions_creation_step_delete)
            {
                taskInfo->done = true;
                bool ret = false;
                MB_String t;
                t += fb_esp_pgm_str_428;
                t += taskInfo->projectId;
                t += fb_esp_pgm_str_431;
                ret = mListOperations(taskInfo->fbdo, toStringPtr(t), toStringPtr("1"), toStringPtr(_EMPTY_STR));
                if (ret)
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;

                    if (!taskInfo->fbdo->session.jsonPtr)
                        taskInfo->fbdo->session.jsonPtr = new FirebaseJson();

                    if (!taskInfo->fbdo->session.dataPtr)
                        taskInfo->fbdo->session.dataPtr = new FirebaseJsonData();

                    taskInfo->fbdo->session.jsonPtr->clear();
                    taskInfo->fbdo->session.jsonPtr->setJsonData(taskInfo->fbdo->session.cfn.payload.c_str());
                    taskInfo->fbdo->session.jsonPtr->get(*taskInfo->fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_432));

                    sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, taskInfo->fbdo->session.dataPtr->to<const char *>(), taskInfo->callback, taskInfo->statusInfo);
                }

                ret = mDeleteFunction(taskInfo->fbdo, toStringPtr(taskInfo->projectId), toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId));
            }

            if (taskInfo->step == fb_esp_functions_creation_step_polling_status)
            {
                if (millis() - _lasPollMs > 5000 || _lasPollMs == 0)
                {
                    _lasPollMs = millis();

                    if (!taskInfo->_delete && !taskInfo->active)
                    {

                        bool ret = mGetFunction(taskInfo->fbdo, toStringPtr(taskInfo->projectId), toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId));

                        if (ret)
                        {
                            if (_function_status == fb_esp_functions_status_UNKNOWN || _function_status == fb_esp_functions_status_OFFLINE)
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
                                sendCallback(taskInfo->fbdo, taskInfo->fbdo->session.cfn.cbInfo.status, taskInfo->fbdo->session.error.c_str(), taskInfo->callback, taskInfo->statusInfo);
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

#endif
}

#endif

#endif // ENABLE