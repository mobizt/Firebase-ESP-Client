/**
 * Google's Cloud Storage class, GCS.cpp version 1.0.4
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

#ifndef GOOGLE_CS_CPP
#define GOOGLE_CS_CPP

#include "GCS.h"

GG_CloudStorage::GG_CloudStorage()
{
}
GG_CloudStorage::~GG_CloudStorage()
{
}

void GG_CloudStorage::begin(UtilsClass *u)
{
    ut = u;
}

bool GG_CloudStorage::upload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, fb_esp_gcs_upload_type uploadType, const char *remoteFileName, const char *mime, UploadOptions *uploadOptions, RequestProperties *requestProps, UploadStatusInfo *status, ProgressCallback callback)
{
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.localFileName = localFileName;
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.mime = mime;
    req.statusInfo = status;
    req.callback = callback;
    req.uploadOptions = uploadOptions;
    req.requestProps = requestProps;
    if (status)
    {
        req.statusInfo->localFileName = localFileName;
    }
    if (uploadType == gcs_upload_type_simple)
        req.requestType = fb_esp_gcs_request_type_upload_simple;
    else if (uploadType == gcs_upload_type_multipart)
        req.requestType = fb_esp_gcs_request_type_upload_multipart;
    else if (uploadType == gcs_upload_type_resumable)
        req.requestType = fb_esp_gcs_request_type_upload_resumable_init;

    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::sendRequest(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect())
        return false;

    if (Signer.config->host.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    if (!Signer.tokenReady())
        return false;

    if (fbdo->_ss.long_running_task > 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Signer.getCfg()->_int.fb_processing)
        return false;

    Signer.getCfg()->_int.fb_processing = true;

    gcs_connect(fbdo);

    fbdo->_ss.gcs.meta.name.clear();
    fbdo->_ss.gcs.meta.bucket.clear();
    fbdo->_ss.gcs.meta.contentType.clear();
    fbdo->_ss.gcs.meta.crc32.clear();
    fbdo->_ss.gcs.meta.etag.clear();
    fbdo->_ss.gcs.meta.downloadTokens.clear();
    fbdo->_ss.gcs.meta.generation = 0;
    fbdo->_ss.gcs.meta.size = 0;

    if (req->requestType == fb_esp_gcs_request_type_download)
    {
        if (req->remoteFileName.length() == 0)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }

        if (req->remoteFileName[0] == '/' && req->remoteFileName.length() == 1)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }
    }

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        if (req->localFileName.length() == 0)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }

        if (req->localFileName[0] == '/' && req->localFileName.length() == 1)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }
    }

    bool ret = gcs_sendRequest(fbdo, req);
    if (!ret && (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init))
    {
        fbdo->_ss.gcs.cbInfo.status = fb_esp_gcs_upload_status_error;
        UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_upload_status_error;
        in.progress = 0;
        in.errorMsg = fbdo->errorReason().c_str();
        sendCallback(fbdo, in, req->callback, req->statusInfo);
    }
    return ret;
}

bool GG_CloudStorage::download(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType, StorageGetOptions *options)
{
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.localFileName = localFileName;
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.getOptions = options;
    req.requestType = fb_esp_gcs_request_type_download;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::deleteFile(FirebaseData *fbdo, const char *bucketID, const char *fileName, DeleteOptions *options)
{
    struct fb_esp_gcs_req_t req;
    req.requestType = fb_esp_gcs_request_type_delete;
    req.remoteFileName = fileName;
    req.bucketID = bucketID;
    req.deleteOptions = options;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::listFiles(FirebaseData *fbdo, const char *bucketID, ListOptions *options)
{
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.listOptions = options;
    req.requestType = fb_esp_gcs_request_type_list;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::getMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, StorageGetOptions *options)
{
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.remoteFileName = remoteFileName;
    req.getOptions = options;
    req.requestType = fb_esp_gcs_request_type_get_metadata;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::gcs_connect(FirebaseData *fbdo)
{
    std::string host;
    ut->appendP(host, fb_esp_pgm_str_120);
    rescon(fbdo, host.c_str());
    fbdo->httpClient.begin(host.c_str(), 443);
    return true;
}

void GG_CloudStorage::rescon(FirebaseData *fbdo, const char *host)
{
    if (!fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.con_mode != fb_esp_con_mode_gc_storage || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->_ss.host = host;
    fbdo->_ss.con_mode = fb_esp_con_mode_gc_storage;
}

void GG_CloudStorage::reportUpploadProgress(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req, size_t readBytes)
{

    int p = 100 * readBytes / req->fileSize;

    if ((p % 2 == 0) && (p <= 100))
    {
        if (req->reportState == 0 || (p == 0 && req->reportState == -1))
        {
            fbdo->_ss.gcs.cbInfo.status = fb_esp_gcs_upload_status_upload;
            UploadStatusInfo in;
            in.localFileName = req->localFileName;
            in.remoteFileName = req->remoteFileName;
            in.status = fb_esp_gcs_upload_status_upload;
            in.progress = p;
            sendCallback(fbdo, in, req->callback, req->statusInfo);

            if (p == 0 && req->reportState == -1)
                req->reportState = 1;
            else if (req->reportState == 0)
                req->reportState = -1;
        }
    }
    else
        req->reportState = 0;
}

bool GG_CloudStorage::gcs_sendRequest(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req)
{

    fbdo->_ss.gcs.requestType = req->requestType;

    std::string token = Signer.getToken(Signer.getTokenType());

    if (!Signer.getCfg()->_int.fb_flash_rdy)
        ut->flashTest();

    if (req->requestType == fb_esp_gcs_request_type_download)
    {
        if (req->storageType == mem_storage_type_sd)
        {
            if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            {
                fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                return false;
            }
            Signer.getCfg()->_int.fb_file = SD_FS.open(req->localFileName.c_str(), FILE_WRITE);
        }
        else if (req->storageType == mem_storage_type_flash)
        {
            if (!Signer.getCfg()->_int.fb_flash_rdy)
                ut->flashTest();
            Signer.getCfg()->_int.fb_file = FLASH_FS.open(req->localFileName.c_str(), "w");
        }
    }
    else
    {
        if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        {
            if (req->storageType == mem_storage_type_sd)
            {
                if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                    return false;
                }

                if (!Signer.getCfg()->_int.fb_sd_rdy)
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                    return false;
                }

                if (!SD_FS.exists(req->localFileName.c_str()))
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                    return false;
                }

                Signer.getCfg()->_int.fb_file = SD_FS.open(req->localFileName.c_str(), FILE_READ);
            }
            else if (req->storageType == mem_storage_type_flash)
            {
                if (!Signer.getCfg()->_int.fb_flash_rdy)
                    ut->flashTest();

                if (!Signer.getCfg()->_int.fb_flash_rdy)
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                    return false;
                }

                if (!FLASH_FS.exists(req->localFileName.c_str()))
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                    return false;
                }

                Signer.getCfg()->_int.fb_file = FLASH_FS.open(req->localFileName.c_str(), "r");
            }

            req->fileSize = Signer.getCfg()->_int.fb_file.size();

            if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
            {
                if (req->fileSize < gcs_min_chunkSize)
                    req->requestType = fb_esp_gcs_request_type_upload_multipart;
                else
                {
                    uint32_t chunkLen = req->fileSize / 4;
                    uint32_t factor = (chunkLen / gcs_min_chunkSize);
                    if (factor * gcs_min_chunkSize < gcs_min_chunkSize)
                        gcs_chunkSize = gcs_min_chunkSize;
                    else
                        gcs_chunkSize = factor * gcs_min_chunkSize;
                }
            }
        }
    }

    std::string header;
    std::string multipart_header;
    std::string multipart_header2;
    std::string payload;
    std::string boundary = ut->getBoundary(15);

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_upload_status_init;
        sendCallback(fbdo, in, req->callback, req->statusInfo);
    }

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        ut->appendP(header, fb_esp_pgm_str_24);
    else if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_list || req->requestType == fb_esp_gcs_request_type_get_metadata)
        ut->appendP(header, fb_esp_pgm_str_25);
    else if (req->requestType == fb_esp_gcs_request_type_delete)
        ut->appendP(header, fb_esp_pgm_str_27);
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
        ut->appendP(header, fb_esp_pgm_str_23);

    ut->appendP(header, fb_esp_pgm_str_6);

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        ut->appendP(header, fb_esp_pgm_str_521);

    if (req->requestType != fb_esp_gcs_request_type_upload_resumable_run)
    {
        ut->appendP(header, fb_esp_pgm_str_520);
        header += req->bucketID;
        ut->appendP(header, fb_esp_pgm_str_522);
    }

    if (req->requestType == fb_esp_gcs_request_type_download)
    {
        ut->appendP(header, fb_esp_pgm_str_1);
        header += ut->url_encode(req->remoteFileName);
        ut->appendP(header, fb_esp_pgm_str_523);
        setGetOptions(req, header, true);
        ut->appendP(header, fb_esp_pgm_str_30);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_simple)
    {
        ut->appendP(header, fb_esp_pgm_str_524);
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        setUploadOptions(req, header, true);
        ut->appendP(header, fb_esp_pgm_str_30);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
    {
        ut->appendP(header, fb_esp_pgm_str_525);
        setUploadOptions(req, header, true);
        ut->appendP(header, fb_esp_pgm_str_30);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        ut->appendP(header, fb_esp_pgm_str_526);
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        setUploadOptions(req, header, true);
        ut->appendP(header, fb_esp_pgm_str_30);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
    {
        fb_esp_url_info_t urlInfo;
        ut->getUrlInfo(req->location, urlInfo);
        ut->appendP(header, fb_esp_pgm_str_1);
        header += urlInfo.uri;
        ut->appendP(header, fb_esp_pgm_str_30);

        ut->appendP(header, fb_esp_pgm_str_31);
        header += urlInfo.host;
        ut->appendP(header, fb_esp_pgm_str_21);
    }
    else if (req->requestType == fb_esp_gcs_request_type_delete)
    {
        ut->appendP(header, fb_esp_pgm_str_1);
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        setDeleteOptions(req, header, true);
        ut->appendP(header, fb_esp_pgm_str_30);
    }
    else if (req->requestType == fb_esp_gcs_request_type_list)
    {
        setListOptions(req, header, false);
        ut->appendP(header, fb_esp_pgm_str_30);
    }
    else if (req->requestType == fb_esp_gcs_request_type_get_metadata)
    {
        ut->appendP(header, fb_esp_pgm_str_1);
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        ut->appendP(header, fb_esp_pgm_str_527);

        setGetOptions(req, header, true);
        ut->appendP(header, fb_esp_pgm_str_30);
    }

    if (req->requestType != fb_esp_gcs_request_type_upload_resumable_run)
    {
        ut->appendP(header, fb_esp_pgm_str_31);
        ut->appendP(header, fb_esp_pgm_str_193);
        ut->appendP(header, fb_esp_pgm_str_4);
        ut->appendP(header, fb_esp_pgm_str_120);
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_237);
        if (Signer.getTokenType() == token_type_oauth2_access_token)
            ut->appendP(header, fb_esp_pgm_str_271);
        header += token;
        ut->appendP(header, fb_esp_pgm_str_21);
    }

    ut->appendP(header, fb_esp_pgm_str_32);
    ut->appendP(header, fb_esp_pgm_str_34);

    if (req->requestType == fb_esp_gcs_request_type_upload_simple)
    {

        ut->appendP(header, fb_esp_pgm_str_8);
        header += req->mime;
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_12);
        char *tmp = ut->intStr(req->fileSize);
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
    {
        ut->appendP(multipart_header, fb_esp_pgm_str_529);
        multipart_header += boundary;
        ut->appendP(multipart_header, fb_esp_pgm_str_21);
        ut->appendP(multipart_header, fb_esp_pgm_str_528);
        ut->appendP(multipart_header, fb_esp_pgm_str_21);

        fbdo->_ss.json.clear();

        char *tmp = ut->strP(fb_esp_pgm_str_274);
        if (req->remoteFileName[0] == '/')
            fbdo->_ss.json.add((const char *)tmp, ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1)).c_str());
        else
            fbdo->_ss.json.add((const char *)tmp, ut->url_encode(req->remoteFileName).c_str());
        ut->delS(tmp);

        tmp = ut->strP(fb_esp_pgm_str_277);
        fbdo->_ss.json.add((const char *)tmp, req->mime.c_str());
        ut->delS(tmp);

        bool hasProps = false;
        setRequestproperties(req, &fbdo->_ss.json, hasProps);

        String s;
        fbdo->_ss.json.toString(s);

        multipart_header += s.c_str();
        ut->appendP(multipart_header, fb_esp_pgm_str_21);
        ut->appendP(multipart_header, fb_esp_pgm_str_21);

        ut->appendP(multipart_header, fb_esp_pgm_str_529);
        multipart_header += boundary;
        ut->appendP(multipart_header, fb_esp_pgm_str_21);
        ut->appendP(multipart_header, fb_esp_pgm_str_21);

        ut->appendP(multipart_header2, fb_esp_pgm_str_21);
        ut->appendP(multipart_header2, fb_esp_pgm_str_529);
        multipart_header2 += boundary;
        ut->appendP(multipart_header2, fb_esp_pgm_str_529);

        ut->appendP(header, fb_esp_pgm_str_8);
        ut->appendP(header, fb_esp_pgm_str_533);
        header += boundary;
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_12);
        tmp = ut->intStr(req->fileSize + multipart_header.length() + multipart_header2.length());
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        ut->appendP(header, fb_esp_pgm_str_530);
        header += req->mime;
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_531);
        char *tmp = ut->intStr(req->fileSize);
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_528);

        fbdo->_ss.json.clear();

        bool hasProps = false;
        setRequestproperties(req, &fbdo->_ss.json, hasProps);
        fbdo->_ss.json.int_tostr(payload);

        ut->appendP(header, fb_esp_pgm_str_12);

        tmp = ut->intStr(payload.length());
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
    {

        ut->appendP(header, fb_esp_pgm_str_12);

        req->chunkPos = req->chunkRange + 1;
        if (req->chunkRange == -1 && req->fileSize <= gcs_chunkSize)
            req->chunkLen = req->fileSize;
        else if (req->chunkRange == -1 && req->fileSize > gcs_chunkSize)
            req->chunkLen = gcs_chunkSize;
        else if (req->chunkRange != -1)
        {
            int b = req->fileSize - req->chunkRange;
            if (b > (int)gcs_chunkSize)
                req->chunkLen = gcs_chunkSize;
            else
                req->chunkLen = b - 1;
        }

        char *tmp = ut->intStr(req->chunkLen);
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);

        if (req->chunkRange != -1 || req->location.length() > 0)
        {
            ut->appendP(header, fb_esp_pgm_str_532);
            tmp = ut->intStr(req->chunkPos);
            header += tmp;
            ut->delS(tmp);
            ut->appendP(header, fb_esp_pgm_str_397);

            tmp = ut->intStr(req->chunkPos + req->chunkLen - 1);
            header += tmp;
            ut->delS(tmp);

            ut->appendP(header, fb_esp_pgm_str_1);

            tmp = ut->intStr(req->fileSize);
            header += tmp;
            ut->delS(tmp);

            ut->appendP(header, fb_esp_pgm_str_21);
        }
    }

    ut->appendP(header, fb_esp_pgm_str_21);

    int ret = fbdo->httpClient.send(header.c_str(), payload.c_str());
    header.clear();
    payload.clear();
    boundary.clear();
    std::string().swap(header);
    std::string().swap(payload);
    std::string().swap(boundary);

    if (ret == 0)
    {
        fbdo->_ss.connected = true;
        if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart)
        {
            fbdo->_ss.long_running_task++;

            if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
            {
                ret = fbdo->httpClient.send(multipart_header.c_str(), "");
                multipart_header.clear();
                std::string().swap(multipart_header);
            }

            int available = req->fileSize;
            size_t byteRead = 0;
            int bufLen = 2048;
            uint8_t *buf = new uint8_t[bufLen + 1];
            size_t read = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                read = Signer.getCfg()->_int.fb_file.read(buf, available);
                byteRead += read;
                reportUpploadProgress(fbdo, req, byteRead);
                if (fbdo->httpClient.stream()->write(buf, read) != read)
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_UPLOAD_DATA_ERRROR;
                    fbdo->closeSession();
                    break;
                }

                available = Signer.getCfg()->_int.fb_file.available();
            }
            delete[] buf;
            fbdo->_ss.long_running_task--;

            Signer.getCfg()->_int.fb_file.close();

            if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
            {
                ret = fbdo->httpClient.send(multipart_header2.c_str(), "");
                multipart_header2.clear();
                std::string().swap(multipart_header2);
            }

            reportUpploadProgress(fbdo, req, req->fileSize);
        }
        else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
        {
            size_t byteRead = 0;
            int available = 0;

            int bufLen = 2048;
            uint8_t *buf = new uint8_t[bufLen + 1];
            size_t read = 0;
            size_t totalBytes = req->fileSize;
            if (req->chunkRange != -1 || req->location.length() > 0)
            {
                byteRead = req->chunkPos;
                Signer.getCfg()->_int.fb_file.seek(req->chunkPos);
                totalBytes = req->chunkPos + req->chunkLen;
            }
            available = totalBytes;

            reportUpploadProgress(fbdo, req, req->chunkPos);

            while (byteRead < totalBytes)
            {
                delay(0);
                if (available > bufLen)
                    available = bufLen;
                read = Signer.getCfg()->_int.fb_file.read(buf, available);
                if (fbdo->httpClient.stream()->write(buf, read) != read)
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_UPLOAD_DATA_ERRROR;
                    fbdo->closeSession();
                    break;
                }
                byteRead += read;
                reportUpploadProgress(fbdo, req, byteRead);
                available = Signer.getCfg()->_int.fb_file.available();
                if (byteRead + available > totalBytes)
                    available = totalBytes - byteRead;
            }
            delete[] buf;
            if (Signer.getCfg()->_int.fb_file.available() == 0)
                Signer.getCfg()->_int.fb_file.close();
        }

        if (fbdo->_ss.connected)
        {
            ret = handleResponse(fbdo, req);
            fbdo->closeSession();
            if (ret)
            {
                if (Signer.getCfg()->_int.fb_file && req->requestType == fb_esp_gcs_request_type_download)
                    Signer.getCfg()->_int.fb_file.close();

                Signer.getCfg()->_int.fb_processing = false;

                if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart)
                {
                    UploadStatusInfo in;
                    in.localFileName = req->localFileName;
                    in.remoteFileName = req->remoteFileName;
                    in.status = fb_esp_gcs_upload_status_complete;
                    in.progress = 100;
                    sendCallback(fbdo, in, req->callback, req->statusInfo);
                }
                else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
                {
                    if (Signer.getCfg()->_int.fb_file.available() == 0)
                    {
                        UploadStatusInfo in;
                        in.localFileName = req->localFileName;
                        in.remoteFileName = req->remoteFileName;
                        in.status = fb_esp_gcs_upload_status_complete;
                        in.progress = 100;
                        sendCallback(fbdo, in, req->callback, req->statusInfo);
                        fbdo->_ss.long_running_task -= _resumableUploadTasks.size();
                        _resumableUploadTasks.clear();
                    }
                }
                return ret;
            }
        }
    }

    if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init || req->requestType == fb_esp_gcs_request_type_upload_resumable_run || req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart)
    {
        UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_upload_status_error;
        in.errorMsg = fbdo->errorReason().c_str();
        sendCallback(fbdo, in, req->callback, req->statusInfo);
        fbdo->_ss.long_running_task -= _resumableUploadTasks.size();
        _resumableUploadTasks.clear();
    }

    if (Signer.getCfg()->_int.fb_file && req->requestType == fb_esp_gcs_request_type_download)
        Signer.getCfg()->_int.fb_file.close();

    Signer.getCfg()->_int.fb_processing = false;

    return false;
}

void GG_CloudStorage::setGetOptions(struct fb_esp_gcs_req_t *req, std::string &header, bool hasParams)
{
    char *tmp = nullptr;
    if (req->getOptions)
    {
        if (strlen(req->getOptions->generation) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_493);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->getOptions->generation);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }
        if (strlen(req->getOptions->ifGenerationMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_494);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->getOptions->ifGenerationMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->getOptions->ifGenerationNotMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_495);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->getOptions->ifGenerationNotMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->getOptions->ifMetagenerationMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_496);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->getOptions->ifMetagenerationMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->getOptions->ifMetagenerationNotMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_497);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->getOptions->ifMetagenerationNotMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->getOptions->projection) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_498);
            header += tmp;
            ut->delS(tmp);
            header += req->getOptions->projection;
        }
    }
}

void GG_CloudStorage::setUploadOptions(struct fb_esp_gcs_req_t *req, std::string &header, bool hasParams)
{
    char *tmp = nullptr;
    if (req->uploadOptions)
    {
        if (strlen(req->uploadOptions->contentEncoding) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_499);
            header += tmp;
            ut->delS(tmp);
            header += req->uploadOptions->contentEncoding;
        }

        if (strlen(req->uploadOptions->ifGenerationMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_494);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->uploadOptions->ifGenerationMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->uploadOptions->ifGenerationNotMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_495);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->uploadOptions->ifGenerationNotMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->uploadOptions->ifMetagenerationMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_496);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->uploadOptions->ifMetagenerationMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->uploadOptions->ifMetagenerationNotMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_497);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->uploadOptions->ifMetagenerationNotMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->uploadOptions->kmsKeyName) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_500);
            header += tmp;
            ut->delS(tmp);
            header += req->uploadOptions->kmsKeyName;
        }

        if (strlen(req->uploadOptions->predefinedAcl) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_501);
            header += tmp;
            ut->delS(tmp);
            header += req->uploadOptions->predefinedAcl;
        }

        if (strlen(req->uploadOptions->projection) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_498);
            header += tmp;
            ut->delS(tmp);
            header += req->uploadOptions->projection;
        }
    }
}

void GG_CloudStorage::setRequestproperties(struct fb_esp_gcs_req_t *req, FirebaseJson *json, bool &hasProps)
{

    char *tmp = ut->strP(fb_esp_pgm_str_514);
    char *tmp2 = ut->strP(fb_esp_pgm_str_518);
    char *tmp3 = ut->strP(fb_esp_pgm_str_519);
    static FirebaseJson js;
    js.clear();

    if (req->requestProps)
    {
        if (strlen(req->requestProps->metadata) > 0)
        {
            hasProps = true;
            js.setJsonData(req->requestProps->metadata);
        }
    }

    js.add((const char *)tmp2, (const char *)tmp3);
    json->add((const char *)tmp, js);
    ut->delS(tmp);
    ut->delS(tmp2);
    ut->delS(tmp3);

    if (req->requestProps)
    {
        if (strlen(req->requestProps->acl) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_504);
            static FirebaseJsonArray arr;
            arr.clear();
            arr.setJsonArrayData(req->requestProps->acl);
            json->add((const char *)tmp, arr);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->cacheControl) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_505);
            json->add((const char *)tmp, req->requestProps->cacheControl);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->contentDisposition) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_506);
            json->add((const char *)tmp, req->requestProps->contentDisposition);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->contentEncoding) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_507);
            json->add((const char *)tmp, req->requestProps->contentEncoding);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->contentLanguage) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_508);
            json->add((const char *)tmp, req->requestProps->contentLanguage);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->contentType) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_509);
            json->add((const char *)tmp, req->requestProps->contentType);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->crc32c) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_510);
            json->add((const char *)tmp, req->requestProps->crc32c);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->customTime) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_511);
            json->add((const char *)tmp, req->requestProps->customTime);
            ut->delS(tmp);
            hasProps = true;
        }

        if (strlen(req->requestProps->eventBasedHold) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_512);
            tmp2 = ut->strP(fb_esp_pgm_str_107);
            if (strcmp(req->requestProps->eventBasedHold, tmp2))
                json->add((const char *)tmp, true);
            else
                json->add((const char *)tmp, false);
            ut->delS(tmp);
            ut->delS(tmp2);
            hasProps = true;
        }

        if (strlen(req->requestProps->md5Hash) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_513);
            json->add((const char *)tmp, req->requestProps->md5Hash);
            ut->delS(tmp);
            hasProps = true;
        }

        if (strlen(req->requestProps->name) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_515);
            json->add((const char *)tmp, req->requestProps->name);
            ut->delS(tmp);
            hasProps = true;
        }
        if (strlen(req->requestProps->storageClass) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_516);
            json->add((const char *)tmp, req->requestProps->storageClass);
            ut->delS(tmp);
            hasProps = true;
        }

        if (strlen(req->requestProps->temporaryHold) > 0)
        {
            tmp = ut->strP(fb_esp_pgm_str_517);
            tmp2 = ut->strP(fb_esp_pgm_str_107);
            if (strcmp(req->requestProps->temporaryHold, tmp2))
                json->add((const char *)tmp, true);
            else
                json->add((const char *)tmp, false);
            ut->delS(tmp);
            ut->delS(tmp2);
            hasProps = true;
        }
    }
}

void GG_CloudStorage::setDeleteOptions(struct fb_esp_gcs_req_t *req, std::string &header, bool hasParams)
{
    char *tmp = nullptr;

    if (req->deleteOptions)
    {
        if (strlen(req->deleteOptions->ifGenerationMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_494);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->deleteOptions->ifGenerationMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->deleteOptions->ifGenerationNotMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_495);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->deleteOptions->ifGenerationNotMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->deleteOptions->ifMetagenerationMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_496);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->deleteOptions->ifMetagenerationMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->deleteOptions->ifMetagenerationNotMatch) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_497);
            header += tmp;
            ut->delS(tmp);

            int n = atoi(req->deleteOptions->ifMetagenerationNotMatch);
            tmp = ut->intStr(n);
            header += tmp;
            ut->delS(tmp);
        }
    }
}

void GG_CloudStorage::setListOptions(struct fb_esp_gcs_req_t *req, std::string &header, bool hasParams)
{

    char *tmp = nullptr;
    char *tmp2 = nullptr;

    if (req->listOptions)
    {
        if (strlen(req->listOptions->delimiter) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_485);
            header += tmp;
            ut->delS(tmp);
            header += req->listOptions->delimiter;
        }

        if (strlen(req->listOptions->endOffset) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_486);
            header += tmp;
            ut->delS(tmp);
            header += req->listOptions->endOffset;
        }

        if (strlen(req->listOptions->includeTrailingDelimiter) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_487);
            header += tmp;
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_107);
            tmp2 = ut->strP(fb_esp_pgm_str_106);

            if (strcmp(req->listOptions->includeTrailingDelimiter, tmp))
                header += tmp;
            else
                header += tmp2;
            ut->delS(tmp);
            ut->delS(tmp2);
        }

        if (strlen(req->listOptions->maxResults) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_484);
            header += tmp;
            ut->delS(tmp);

            int m = atoi(req->listOptions->maxResults);
            tmp = ut->intStr(m);
            header += tmp;
            ut->delS(tmp);
        }

        if (strlen(req->listOptions->pageToken) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_358);
            tmp2 = ut->strP(fb_esp_pgm_str_361);
            header += tmp;
            header += tmp2;
            header += req->listOptions->pageToken;
            ut->delS(tmp);
            ut->delS(tmp2);
        }

        if (strlen(req->listOptions->prefix) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_488);
            header += tmp;
            ut->delS(tmp);
            header += req->listOptions->prefix;
        }

        if (strlen(req->listOptions->projection) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_489);
            header += tmp;
            ut->delS(tmp);
            header += req->listOptions->projection;
        }

        if (strlen(req->listOptions->startOffset) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_490);
            header += tmp;
            ut->delS(tmp);
            header += req->listOptions->startOffset;
        }

        if (strlen(req->listOptions->versions) > 0)
        {
            if (hasParams)
            {
                tmp = ut->strP(fb_esp_pgm_str_172);
                header += tmp;
            }
            else
            {
                hasParams = true;
                tmp = ut->strP(fb_esp_pgm_str_173);
                header += tmp;
            }
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_491);
            header += tmp;
            ut->delS(tmp);

            tmp = ut->strP(fb_esp_pgm_str_107);
            tmp2 = ut->strP(fb_esp_pgm_str_106);
            if (strcmp(req->listOptions->versions, tmp))
                header += tmp;
            else
                header += tmp2;
            ut->delS(tmp);
            ut->delS(tmp2);
        }
    }
}

void GG_CloudStorage::sendCallback(FirebaseData *fbdo, UploadStatusInfo &in, ProgressCallback cb, UploadStatusInfo *out)
{

    fbdo->_ss.gcs.cbInfo.status = in.status;
    fbdo->_ss.gcs.cbInfo.errorMsg = in.errorMsg;
    fbdo->_ss.gcs.cbInfo.progress = in.progress;
    fbdo->_ss.gcs.cbInfo.localFileName = in.localFileName;
    fbdo->_ss.gcs.cbInfo.remoteFileName = in.remoteFileName;

    if (cb)
        cb(fbdo->_ss.gcs.cbInfo);

    if (out)
    {
        out->errorMsg = fbdo->_ss.gcs.cbInfo.errorMsg;
        out->status = fbdo->_ss.gcs.cbInfo.status;
        out->progress = fbdo->_ss.gcs.cbInfo.progress;
        out->localFileName = fbdo->_ss.gcs.cbInfo.localFileName;
    }
}

#if defined(ESP32)
void GG_CloudStorage::runResumableUploadTask(const char *taskName)
#elif defined(ESP8266)
void GG_CloudStorage::runResumableUploadTask()
#endif
{
#if defined(ESP32)

    static GG_CloudStorage *_this = this;

    TaskFunction_t taskCode = [](void *param) {
        while (_this->_resumable_upload_task_enable)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);

            if (_this->_resumableUploadTasks.size() == 0)
                break;

            if (_this->_resumableUplaodTaskIndex < _this->_resumableUploadTasks.size() - 1)
                _this->_resumableUplaodTaskIndex++;
            else
                _this->_resumableUplaodTaskIndex = 0;

            struct fb_gcs_upload_resumable_task_info_t *taskInfo = &_this->_resumableUploadTasks[_this->_resumableUplaodTaskIndex];

            if (!taskInfo->done)
            {
                taskInfo->done = true;
                _this->gcs_sendRequest(taskInfo->fbdo, &taskInfo->req);
            }

            size_t n = 0;
            for (size_t i = 0; i < _this->_resumableUploadTasks.size(); i++)
                if (_this->_resumableUploadTasks[i].done)
                    n++;

            if (n == _this->_resumableUploadTasks.size())
            {
                _this->_resumableUploadTasks.clear();
                _this->_resumable_upload_task_enable = false;
            }

            yield();
        }

        Signer.getCfg()->_int.resumable_upload_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName, 12000, NULL, 3, &Signer.getCfg()->_int.resumable_upload_task_handle, 1);

#elif defined(ESP8266)

    if (_resumable_upload_task_enable)
    {
        delay(100);

        if (_resumableUploadTasks.size() == 0)
            return;

        if (_resumableUplaodTaskIndex < _resumableUploadTasks.size() - 1)
            _resumableUplaodTaskIndex++;
        else
            _resumableUplaodTaskIndex = 0;

        struct fb_gcs_upload_resumable_task_info_t *taskInfo = &_resumableUploadTasks[_resumableUplaodTaskIndex];

        if (!taskInfo->done)
        {
            taskInfo->done = true;
            gcs_sendRequest(taskInfo->fbdo, &taskInfo->req);
        }

        size_t n = 0;
        for (size_t i = 0; i < _resumableUploadTasks.size(); i++)
            if (_resumableUploadTasks[i].done)
                n++;

        if (_resumableUploadTasks.size() > 0 && n < _resumableUploadTasks.size())
            ut->set_scheduled_callback(std::bind(&GG_CloudStorage::runResumableUploadTask, this));

        if (n == _resumableUploadTasks.size())
        {
            _resumableUploadTasks.clear();
            _resumable_upload_task_enable = false;
        }
    }

#endif
}

bool GG_CloudStorage::handleResponse(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req)
{

    if (!fbdo->reconnect())
        return false;

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->httpClient.stream();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    char *header = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->_ss.resp_size;
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
    std::string part1, part2, part3, part4, part5;
    size_t p1 = 0;
    size_t p2 = 0;

    char *tmp1 = nullptr;
    char *tmp2 = nullptr;
    char *tmp3 = nullptr;
    char *tmp4 = nullptr;
    char *tmp5 = nullptr;

    if (req->requestType == fb_esp_gcs_request_type_list)
    {
        tmp1 = ut->strP(fb_esp_pgm_str_476);
        tmp2 = ut->strP(fb_esp_pgm_str_477);
        tmp3 = ut->strP(fb_esp_pgm_str_482);
        tmp4 = ut->strP(fb_esp_pgm_str_483);
        tmp5 = ut->strP(fb_esp_pgm_str_3);
    }

    std::string payload;

    fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->_ss.content_length = -1;
    fbdo->_ss.rtdb.data_mismatch = false;
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    defaultChunkSize = 2048;

    while (fbdo->httpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = stream->available();
        delay(0);
    }

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
                    header = ut->newS(chunkBufSize);
                    hstate = 1;
                    int readLen = ut->readLine(stream, header, chunkBufSize);
                    int pos = 0;

                    tmp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    delay(0);
                    dataTime = millis();
                    if (tmp)
                    {
                        //http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(tmp);
                        if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK || response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT)
                            fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
                        else
                            fbdo->_ss.http_code = response.httpCode;
                        ut->delS(tmp);
                    }
                }
                else
                {
                    delay(0);
                    dataTime = millis();
                    //the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        //read one line of next header field until the empty header has found
                        tmp = ut->newS(chunkBufSize);
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
                            //parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);
                            fbdo->_ss.chunked_encoding = response.isChunkedEnc;

                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK || response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT)
                                error.code = 0;

                            if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init || req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
                            {
                                if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT) //resume incomplete
                                {
                                    int p1 = ut->strposP(header, fb_esp_pgm_str_481, 0);
                                    if (p1 > -1)
                                    {
                                        if (_resumableUploadTasks.size() > 0)
                                        {
                                            struct fb_gcs_upload_resumable_task_info_t ruTask;
                                            ruTask.req.fileSize = req->fileSize;
                                            ruTask.req.location = req->location;
                                            ruTask.req.localFileName = req->localFileName;
                                            ruTask.req.remoteFileName = req->remoteFileName;
                                            ruTask.fbdo = fbdo;
                                            ruTask.req.requestType = fb_esp_gcs_request_type_upload_resumable_run;

                                           char *tmp6 = ut->newS(strlen(header));
                                           strncpy(tmp6, header + p1 + strlen_P(fb_esp_pgm_str_481), strlen(header) - p1 - strlen_P(fb_esp_pgm_str_481));
                                           ruTask.req.chunkRange = atoi(tmp6);
                                           ut->delS(tmp6);

                                           ruTask.req.callback = req->callback;
                                           ruTask.req.statusInfo = req->statusInfo;

                                           _resumableUploadTasks.push_back(ruTask);

                                           fbdo->_ss.long_running_task++;
                                           _resumable_upload_task_enable = true;

                                           if (_resumableUploadTasks.size() == 1)
                                           {
#if defined(ESP32)
                                               tmp6 = ut->strP(fb_esp_pgm_str_480);
                                               runResumableUploadTask(tmp6);
                                               ut->delS(tmp6);
#elif defined(ESP8266)
                                                runResumableUploadTask();
#endif
                                            }
                                        }
                                    }
                                }

                                if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.location.length() > 0)
                                {
                                    struct fb_gcs_upload_resumable_task_info_t ruTask;
                                    ruTask.req.fileSize = req->fileSize;
                                    ruTask.fbdo = fbdo;
                                    ruTask.req.location = response.location;
                                    ruTask.req.localFileName = req->localFileName;
                                    ruTask.req.remoteFileName = req->remoteFileName;
                                    ruTask.req.requestType = fb_esp_gcs_request_type_upload_resumable_run;

                                    ruTask.req.callback = req->callback;
                                    ruTask.req.statusInfo = req->statusInfo;

                                    _resumableUploadTasks.push_back(ruTask);

                                    fbdo->_ss.long_running_task++;
                                    _resumable_upload_task_enable = true;

                                    if (_resumableUploadTasks.size() == 1)
                                    {
#if defined(ESP32)
                                        char *tmp7 = ut->strP(fb_esp_pgm_str_480);
                                        runResumableUploadTask(tmp7);
                                        ut->delS(tmp7);
#elif defined(ESP8266)
                                        runResumableUploadTask();
#endif
                                    }
                                }
                            }

                            if (hstate == 1)
                                ut->delS(header);
                            hstate = 0;

                            if (response.contentLen == 0)
                            {
                                ut->delS(tmp);
                                break;
                            }
                        }
                        else
                        {
                            //accumulate the remaining header field
                            memcpy(header + hBufPos, tmp, readLen);
                            hBufPos += readLen;
                        }
                        ut->delS(tmp);
                    }
                    else
                    {
                        //the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.contentLen > 0 && req->requestType == fb_esp_gcs_request_type_download)
                            {
                                size_t available = fbdo->httpClient.stream()->available();
                                uint8_t *buf = new uint8_t[defaultChunkSize + 1];
                                while (available > 0)
                                {
                                    if (available > defaultChunkSize)
                                        available = defaultChunkSize;

                                    size_t read = fbdo->httpClient.stream()->read(buf, available);
                                    if (read == available)
                                        Signer.getCfg()->_int.fb_file.write(buf, read);
                                    available = fbdo->httpClient.stream()->available();
                                    payloadRead += available;
                                }
                                error.code = response.contentLen = payloadRead;
                                delete[] buf;
                                break;
                            }
                            else
                            {
                                pChunkIdx++;
                                pChunk = ut->newS(chunkBufSize + 1);

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
                                    payloadRead += availablePayload;
                                    if (req->requestType == fb_esp_gcs_request_type_list)
                                    {
                                        delay(10);
                                        part1 = pChunk;
                                        p1 = part1.find(tmp1);
                                        if (p1 != std::string::npos)
                                        {
                                            p2 = part1.find(tmp5, p1 + strlen(tmp1));
                                            if (p2 != std::string::npos)
                                                part2 = part1.substr(p1 + strlen(tmp1), p2 - p1 - strlen(tmp1));
                                        }

                                        if (part2.length() > 1)
                                        {
                                            if (part2[part2.length() - 1] != '/')
                                            {

                                                p1 = part1.find(tmp2);
                                                if (p1 != std::string::npos)
                                                {
                                                    p2 = part1.find(tmp5, p1 + strlen(tmp2));
                                                    if (p2 != std::string::npos)
                                                        part3 = part1.substr(p1 + strlen(tmp2), p2 - p1 - strlen(tmp2));
                                                }

                                                p1 = part1.find(tmp3);
                                                if (p1 != std::string::npos)
                                                {
                                                    p2 = part1.find(tmp5, p1 + strlen(tmp3));
                                                    if (p2 != std::string::npos)
                                                        part4 = part1.substr(p1 + strlen(tmp3), p2 - p1 - strlen(tmp3));
                                                }

                                                p1 = part1.find(tmp4);
                                                if (p1 != std::string::npos)
                                                {
                                                    p2 = part1.find(tmp5, p1 + strlen(tmp4));
                                                    if (p2 != std::string::npos)
                                                    {
                                                        part5 = part1.substr(p1 + strlen(tmp4), p2 - p1 - strlen(tmp4));
                                                        fb_esp_fcs_file_list_item_t itm;
                                                        itm.name = part2;
                                                        itm.bucket = part3;
                                                        itm.contentType = part4;
                                                        itm.size = atoi(part5.c_str());
                                                        fbdo->_ss.fcs.files.items.push_back(itm);
                                                        part1.clear();
                                                        part2.clear();
                                                        part3.clear();
                                                        part4.clear();
                                                        part5.clear();
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    if (payloadRead < payloadLen)
                                        payload += pChunk;
                                }

                                ut->delS(pChunk);

                                if (availablePayload < 0 || (payloadRead >= response.contentLen && !response.isChunkedEnc))
                                {
                                    while (stream->available() > 0)
                                        stream->read();
                                    break;
                                }
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

        if (req->requestType == fb_esp_gcs_request_type_list)
        {
            ut->delS(tmp1);
            ut->delS(tmp2);
            ut->delS(tmp3);
            ut->delS(tmp4);
            ut->delS(tmp5);
        }

        if (hstate == 1)
            ut->delS(header);

        //parse the payload
        if (payload.length() > 0)
        {
            if (payload[0] == '{')
            {
                fbdo->_ss.json.setJsonData(payload.c_str());
                payload.clear();
                std::string().swap(payload);

                char *tmp = ut->strP(fb_esp_pgm_str_257);
                fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                ut->delS(tmp);

                if (fbdo->_ss.data.success)
                {
                    error.code = fbdo->_ss.data.intValue;
                    tmp = ut->strP(fb_esp_pgm_str_258);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.error = fbdo->_ss.data.stringValue.c_str();
                }
                else
                {
                    error.code = 0;

                    tmp = ut->strP(fb_esp_pgm_str_274);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.name = fbdo->_ss.data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_275);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.bucket = fbdo->_ss.data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_276);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.generation = atoi(fbdo->_ss.data.stringValue.c_str());

                    tmp = ut->strP(fb_esp_pgm_str_503);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.metageneration = atoi(fbdo->_ss.data.stringValue.c_str());

                    tmp = ut->strP(fb_esp_pgm_str_277);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.contentType = fbdo->_ss.data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_278);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.size = atoi(fbdo->_ss.data.stringValue.c_str());

                    tmp = ut->strP(fb_esp_pgm_str_279);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.etag = fbdo->_ss.data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_478);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.crc32 = fbdo->_ss.data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_479);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.downloadTokens = fbdo->_ss.data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_492);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.data.success)
                        fbdo->_ss.gcs.meta.mediaLink = fbdo->_ss.data.stringValue.c_str();
                }
            }

            fbdo->_ss.content_length = response.payloadLen;

            fbdo->_ss.json.clear();
            fbdo->_ss.arr.clear();
        }

        return error.code == 0;
    }
    else
    {

        while (stream->available() > 0)
            stream->read();
    }

    return false;
}

#endif