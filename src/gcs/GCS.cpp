/**
 * Google's Cloud Storage class, GCS.cpp version 1.2.2
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 15, 2022
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

#ifdef ENABLE_GC_STORAGE

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

bool GG_CloudStorage::mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr localFileName, fb_esp_mem_storage_type storageType, fb_esp_gcs_upload_type uploadType, MB_StringPtr remoteFileName, MB_StringPtr mime, UploadOptions *uploadOptions, RequestProperties *requestProps, UploadStatusInfo *status, UploadProgressCallback callback)
{
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.localFileName = localFileName;
    ut->makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.mime = mime;
    req.uploadStatusInfo = status;
    req.uploadCallback = callback;
    req.uploadOptions = uploadOptions;
    req.requestProps = requestProps;
    if (status)
    {
        req.uploadStatusInfo->localFileName = localFileName;
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

    if (fbdo->session.long_running_task > 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Signer.getCfg()->internal.fb_processing)
        return false;

    gcs_connect(fbdo);

    fbdo->session.gcs.meta.name.clear();
    fbdo->session.gcs.meta.bucket.clear();
    fbdo->session.gcs.meta.contentType.clear();
    fbdo->session.gcs.meta.crc32.clear();
    fbdo->session.gcs.meta.etag.clear();
    fbdo->session.gcs.meta.downloadTokens.clear();
    fbdo->session.gcs.meta.generation = 0;
    fbdo->session.gcs.meta.size = 0;

    if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota)
    {
        if (req->remoteFileName.length() == 0)
        {
            fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }

        if (req->remoteFileName[0] == '/' && req->remoteFileName.length() == 1)
        {
            fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }
    }

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        if (req->localFileName.length() == 0)
        {
            fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }

        if (req->localFileName[0] == '/' && req->localFileName.length() == 1)
        {
            fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_NOT_FOUND;
            return false;
        }
    }

    Signer.getCfg()->internal.fb_processing = true;

    bool ret = gcs_sendRequest(fbdo, req);

    Signer.getCfg()->internal.fb_processing = false;

    if (!ret)
    {
        if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota)
        {
            fbdo->session.gcs.cbDownloadInfo.status = fb_esp_gcs_download_status_error;
            DownloadStatusInfo in;
            in.localFileName = req->localFileName;
            in.remoteFileName = req->remoteFileName;
            in.status = fb_esp_gcs_download_status_error;
            in.progress = 0;
            in.errorMsg = fbdo->errorReason().c_str();
            sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
        }
        else if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        {
            fbdo->session.gcs.cbUploadInfo.status = fb_esp_gcs_upload_status_error;
            UploadStatusInfo in;
            in.localFileName = req->localFileName;
            in.remoteFileName = req->remoteFileName;
            in.status = fb_esp_gcs_upload_status_error;
            in.progress = 0;
            in.errorMsg = fbdo->errorReason().c_str();
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        }
    }
    return ret;
}

bool GG_CloudStorage::mDownload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName, MB_StringPtr localFileName, fb_esp_mem_storage_type storageType, StorageGetOptions *options, DownloadProgressCallback callback)
{
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.localFileName = localFileName;
    ut->makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.getOptions = options;
    req.downloadCallback = callback;
    req.requestType = fb_esp_gcs_request_type_download;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::mDownloadOTA(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName, DownloadProgressCallback callback)
{
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.remoteFileName = remoteFileName;
    req.requestType = fb_esp_gcs_request_type_download_ota;
    req.downloadCallback = callback;

    fbdo->closeSession();

#if defined(ESP8266)
    int rx_size = fbdo->session.bssl_rx_size;
    fbdo->session.bssl_rx_size = 16384;
#endif

    bool ret = sendRequest(fbdo, &req);

#if defined(ESP8266)
    fbdo->session.bssl_rx_size = rx_size;
#endif

    fbdo->closeSession();

    return ret;

#endif
    return false;
}

bool GG_CloudStorage::mDeleteFile(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr fileName, DeleteOptions *options)
{
    struct fb_esp_gcs_req_t req;
    req.requestType = fb_esp_gcs_request_type_delete;
    req.remoteFileName = fileName;
    req.bucketID = bucketID;
    req.deleteOptions = options;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::mListFiles(FirebaseData *fbdo, MB_StringPtr bucketID, ListOptions *options)
{
    struct fb_esp_gcs_req_t req;
    req.bucketID = bucketID;
    req.listOptions = options;
    req.requestType = fb_esp_gcs_request_type_list;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::mGetMetadata(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName, StorageGetOptions *options)
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
    MB_String host = fb_esp_pgm_str_120;
    rescon(fbdo, host.c_str());
    fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
    return true;
}

void GG_CloudStorage::rescon(FirebaseData *fbdo, const char *host)
{
    if (fbdo->session.cert_updated || !fbdo->session.connected || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout || fbdo->session.con_mode != fb_esp_con_mode_gc_storage || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->session.host = host;
    fbdo->session.con_mode = fb_esp_con_mode_gc_storage;
}

void GG_CloudStorage::reportUploadProgress(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req, size_t readBytes)
{
    if (req->fileSize == 0)
        return;

    int p = (float)readBytes / req->fileSize * 100;

    if (readBytes == 0)
        fbdo->tcpClient.dataStart = millis();

    if (req->progress != p && (p == 0 || p == 100 || req->progress + ESP_REPORT_PROGRESS_INTERVAL <= p))
    {
        req->progress = p;

        fbdo->tcpClient.dataTime = millis() - fbdo->tcpClient.dataStart;

        fbdo->session.gcs.cbUploadInfo.status = fb_esp_gcs_upload_status_upload;
        UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_upload_status_upload;
        in.progress = p;
        in.elapsedTime = fbdo->tcpClient.dataTime;
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }
}

void GG_CloudStorage::reportDownloadProgress(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req, size_t readBytes)
{
    if (req->fileSize == 0)
        return;

    int p = (float)readBytes / req->fileSize * 100;

    if (readBytes == 0)
        fbdo->tcpClient.dataStart = millis();

    if (req->progress != p && (p == 0 || p == 100 || req->progress + ESP_REPORT_PROGRESS_INTERVAL <= p))
    {
        req->progress = p;

        fbdo->tcpClient.dataTime = millis() - fbdo->tcpClient.dataStart;

        fbdo->session.gcs.cbDownloadInfo.status = fb_esp_gcs_download_status_download;
        DownloadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_download_status_download;
        in.progress = p;
        in.fileSize = req->fileSize;
        in.elapsedTime = fbdo->tcpClient.dataTime;
        sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
    }
}

bool GG_CloudStorage::gcs_sendRequest(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req)
{

    fbdo->session.gcs.requestType = req->requestType;

    int ret = 0;

    if (req->requestType != fb_esp_gcs_request_type_download)
    {
        if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        {

            ret = ut->mbfs->open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);

            // Close file and open later.
            // This is inefficient unless less memory usage than keep file opened
            // which causes the issue in ESP32 core 2.0.x
            if (ret > 0)
                ut->mbfs->close(mbfs_type req->storageType);
            else if (ret < 0)
            {
                fbdo->session.response.code = ret;
                return false;
            }

            req->fileSize = ret;

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

    MB_String header;
    MB_String multipart_header;
    MB_String multipart_header2;
    MB_String boundary = ut->getBoundary(15);

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_upload_status_init;
        in.fileSize = req->fileSize;
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        header += fb_esp_pgm_str_24;
    else if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota || req->requestType == fb_esp_gcs_request_type_list || req->requestType == fb_esp_gcs_request_type_get_metadata)
        header += fb_esp_pgm_str_25;
    else if (req->requestType == fb_esp_gcs_request_type_delete)
        header += fb_esp_pgm_str_27;
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
        header += fb_esp_pgm_str_23;

    header += fb_esp_pgm_str_6;

    if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart || req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        header += fb_esp_pgm_str_521;

    if (req->requestType != fb_esp_gcs_request_type_upload_resumable_run)
    {
        header += fb_esp_pgm_str_520;
        header += req->bucketID;
        header += fb_esp_pgm_str_522;
    }

    if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota)
    {
        header += fb_esp_pgm_str_1;
        header += ut->url_encode(req->remoteFileName);
        header += fb_esp_pgm_str_523;
        setGetOptions(req, header, true);
        header += fb_esp_pgm_str_30;
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_simple)
    {
        header += fb_esp_pgm_str_524;
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        setUploadOptions(req, header, true);
        header += fb_esp_pgm_str_30;
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
    {
        header += fb_esp_pgm_str_525;
        setUploadOptions(req, header, true);
        header += fb_esp_pgm_str_30;
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        header += fb_esp_pgm_str_526;
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        setUploadOptions(req, header, true);
        header += fb_esp_pgm_str_30;
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
    {
        fb_esp_url_info_t urlInfo;
        ut->getUrlInfo(req->location, urlInfo);
        header += fb_esp_pgm_str_1;
        header += urlInfo.uri;
        header += fb_esp_pgm_str_30;

        header += fb_esp_pgm_str_31;
        header += urlInfo.host;
        header += fb_esp_pgm_str_21;
    }
    else if (req->requestType == fb_esp_gcs_request_type_delete)
    {
        header += fb_esp_pgm_str_1;
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        setDeleteOptions(req, header, true);
        header += fb_esp_pgm_str_30;
    }
    else if (req->requestType == fb_esp_gcs_request_type_list)
    {
        setListOptions(req, header, false);
        header += fb_esp_pgm_str_30;
    }
    else if (req->requestType == fb_esp_gcs_request_type_get_metadata)
    {
        header += fb_esp_pgm_str_1;
        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);

        header += fb_esp_pgm_str_527;

        setGetOptions(req, header, true);
        header += fb_esp_pgm_str_30;
    }

    if (req->requestType != fb_esp_gcs_request_type_upload_resumable_run)
    {
        header += fb_esp_pgm_str_31;
        header += fb_esp_pgm_str_193;
        header += fb_esp_pgm_str_4;
        header += fb_esp_pgm_str_120;

        header += fb_esp_pgm_str_21;

        if (!Signer.getCfg()->signer.test_mode)
        {
            header += fb_esp_pgm_str_237;
            if (Signer.getTokenType() == token_type_oauth2_access_token)
                header += fb_esp_pgm_str_209;

            fbdo->tcpClient.send(header.c_str());
            header.clear();

            if (fbdo->session.response.code < 0)
                return false;

            fbdo->tcpClient.send(Signer.getToken());

            if (fbdo->session.response.code < 0)
                return false;

            header += fb_esp_pgm_str_21;
        }
    }

    header += fb_esp_pgm_str_32;
    header += fb_esp_pgm_str_34;

    if (req->requestType == fb_esp_gcs_request_type_upload_simple)
    {

        header += fb_esp_pgm_str_8;
        header += req->mime;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_12;
        header += req->fileSize;
        header += fb_esp_pgm_str_21;
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
    {
        multipart_header += fb_esp_pgm_str_529;
        multipart_header += boundary;
        multipart_header += fb_esp_pgm_str_21;
        multipart_header += fb_esp_pgm_str_528;
        multipart_header += fb_esp_pgm_str_21;

        fbdo->session.jsonPtr->clear();

        if (req->remoteFileName[0] == '/')
            fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_274), ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1)).c_str());
        else
            fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_274), ut->url_encode(req->remoteFileName).c_str());

        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_277), req->mime.c_str());

        bool hasProps = false;
        setRequestproperties(req, fbdo->session.jsonPtr, hasProps);

        multipart_header += fbdo->session.jsonPtr->raw();
        multipart_header += fb_esp_pgm_str_21;
        multipart_header += fb_esp_pgm_str_21;

        multipart_header += fb_esp_pgm_str_529;
        multipart_header += boundary;
        multipart_header += fb_esp_pgm_str_21;
        multipart_header += fb_esp_pgm_str_21;

        multipart_header2 += fb_esp_pgm_str_21;
        multipart_header2 += fb_esp_pgm_str_529;
        multipart_header2 += boundary;
        multipart_header2 += fb_esp_pgm_str_529;

        header += fb_esp_pgm_str_8;
        header += fb_esp_pgm_str_533;
        header += boundary;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_12;
        header += req->fileSize + multipart_header.length() + multipart_header2.length();
        header += fb_esp_pgm_str_21;
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
    {
        header += fb_esp_pgm_str_530;
        header += req->mime;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_531;
        header += req->fileSize;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_528;

        fbdo->session.jsonPtr->clear();

        bool hasProps = false;
        setRequestproperties(req, fbdo->session.jsonPtr, hasProps);

        header += fb_esp_pgm_str_12;

        header += strlen(fbdo->session.jsonPtr->raw());
        header += fb_esp_pgm_str_21;
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
    {

        header += fb_esp_pgm_str_12;

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

        header += req->chunkLen;
        header += fb_esp_pgm_str_21;

        if (req->chunkRange != -1 || req->location.length() > 0)
        {
            header += fb_esp_pgm_str_532;
            header += req->chunkPos;
            header += fb_esp_pgm_str_397;

            header += req->chunkPos + req->chunkLen - 1;

            header += fb_esp_pgm_str_1;
            header += req->fileSize;

            header += fb_esp_pgm_str_21;
        }
    }

    header += fb_esp_pgm_str_21;

    if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota)
    {
        fbdo->tcpClient.send(header.c_str());
        header.clear();
        if (fbdo->session.response.code < 0)
            return false;
    }
    else
    {
        fbdo->tcpClient.send(header.c_str());
        header.clear();
        if (fbdo->session.response.code < 0)
            return false;
        if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init)
        {
            if (fbdo->session.jsonPtr)
                fbdo->tcpClient.send(fbdo->session.jsonPtr->raw());

            if (fbdo->session.response.code < 0)
                return false;
        }
    }

    boundary.clear();

    if (fbdo->session.response.code > 0)
    {

        fbdo->session.gcs.storage_type = req->storageType;
        fbdo->session.connected = true;
        if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart)
        {
            fbdo->session.long_running_task++;

            if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
            {
                fbdo->tcpClient.send(multipart_header.c_str());
                multipart_header.clear();

                if (fbdo->session.response.code < 0)
                    return false;
            }

            int available = req->fileSize;
            size_t byteRead = 0;
            int bufLen = Signer.getCfg()->gcs.upload_buffer_size;
            if (bufLen < 512)
                bufLen = 512;

            if (bufLen > 1024 * 16)
                bufLen = 1024 * 16;

            uint8_t *buf = (uint8_t *)ut->newP(bufLen + 1, false);
            int read = 0;

            // This is inefficient unless less memory usage than keep file opened
            // which causes the issue in ESP32 core 2.0.x
            ut->mbfs->open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                read = ut->mbfs->read(mbfs_type req->storageType, buf, available);

                byteRead += read;

                reportUploadProgress(fbdo, req, byteRead);

                if (fbdo->tcpClient.write(buf, read) != read)
                {
                    fbdo->session.response.code = FIREBASE_ERROR_UPLOAD_DATA_ERRROR;
                    fbdo->closeSession();
                    break;
                }

                available = ut->mbfs->available(mbfs_type req->storageType);
            }

            ut->delP(&buf);
            fbdo->session.long_running_task--;

            ut->mbfs->close(mbfs_type req->storageType);

            if (req->requestType == fb_esp_gcs_request_type_upload_multipart)
            {
                fbdo->tcpClient.send(multipart_header2.c_str());
                multipart_header2.clear();

                if (fbdo->session.response.code < 0)
                    return false;
            }

            reportUploadProgress(fbdo, req, req->fileSize);
        }
        else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
        {
            size_t byteRead = 0;
            int available = 0;

            int bufLen = Signer.getCfg()->gcs.upload_buffer_size;
            if (bufLen < 512)
                bufLen = 512;

            if (bufLen > 1024 * 16)
                bufLen = 1024 * 16;

            uint8_t *buf = (uint8_t *)ut->newP(bufLen + 1);
            int read = 0;
            size_t totalBytes = req->fileSize;

            if (req->chunkRange != -1 || req->location.length() > 0)
            {
                // This is inefficient unless less memory usage than keep file opened
                // which causes the issue in ESP32 core 2.0.x
                ut->mbfs->close(mbfs_type req->storageType);
                ut->mbfs->open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);
                byteRead = req->chunkPos;
                ut->mbfs->seek(mbfs_type req->storageType, req->chunkPos);
                totalBytes = req->chunkPos + req->chunkLen;
            }
            available = totalBytes;

            reportUploadProgress(fbdo, req, req->chunkPos);

            while (byteRead < totalBytes)
            {
                ut->idle();
                if (available > bufLen)
                    available = bufLen;
                read = ut->mbfs->read(mbfs_type req->storageType, buf, available);

                if (fbdo->tcpClient.connected())
                {
                    if (fbdo->tcpClient.write(buf, read) != read)
                    {
                        fbdo->session.response.code = FIREBASE_ERROR_UPLOAD_DATA_ERRROR;
                        fbdo->closeSession();
                        break;
                    }
                }
                else
                {
                    fbdo->session.response.code = FIREBASE_ERROR_UPLOAD_DATA_ERRROR;
                    fbdo->closeSession();
                    break;
                }

                byteRead += read;

                reportUploadProgress(fbdo, req, byteRead);

                available = ut->mbfs->available(mbfs_type req->storageType);
                if (byteRead + available > totalBytes)
                    available = totalBytes - byteRead;
            }

            ut->delP(&buf);

            if (ut->mbfs->available(mbfs_type req->storageType) == 0)
                ut->mbfs->close(mbfs_type req->storageType);
        }

        if (fbdo->session.connected)
        {
            bool ret = handleResponse(fbdo, req);
            fbdo->closeSession();
            if (ret)
            {
                if (ut->mbfs->ready(mbfs_type req->storageType) && req->requestType == fb_esp_gcs_request_type_download)
                    ut->mbfs->close(mbfs_type req->storageType);

                Signer.getCfg()->internal.fb_processing = false;
                if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota)
                {
                    DownloadStatusInfo in;
                    in.localFileName = req->localFileName;
                    in.remoteFileName = req->remoteFileName;
                    in.status = fb_esp_gcs_download_status_complete;
                    in.fileSize = req->fileSize;
                    in.progress = 100;
                    sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
                }
                else if (req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart)
                {
                    UploadStatusInfo in;
                    in.localFileName = req->localFileName;
                    in.remoteFileName = req->remoteFileName;
                    in.status = fb_esp_gcs_upload_status_complete;
                    in.progress = 100;
                    sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
                }
                else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
                {
                    if (ut->mbfs->available(mbfs_type req->storageType) == 0)
                    {
                        UploadStatusInfo in;
                        in.localFileName = req->localFileName;
                        in.remoteFileName = req->remoteFileName;
                        in.status = fb_esp_gcs_upload_status_complete;
                        in.progress = 100;
                        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
                        fbdo->session.long_running_task -= _resumableUploadTasks.size();
                        _resumableUploadTasks.clear();
                    }
                }
                return true;
            }
        }
    }

    if (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota)
    {
        DownloadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_download_status_error;
        in.errorMsg = fbdo->errorReason().c_str();
        sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
    }
    else if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init || req->requestType == fb_esp_gcs_request_type_upload_resumable_run || req->requestType == fb_esp_gcs_request_type_upload_simple || req->requestType == fb_esp_gcs_request_type_upload_multipart)
    {
        UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_gcs_upload_status_error;
        in.errorMsg = fbdo->errorReason().c_str();
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        fbdo->session.long_running_task -= _resumableUploadTasks.size();
        _resumableUploadTasks.clear();
    }

    if (ut->mbfs->ready(mbfs_type req->storageType) && req->requestType == fb_esp_gcs_request_type_download)
        ut->mbfs->close(mbfs_type req->storageType);

    Signer.getCfg()->internal.fb_processing = false;

    return false;
}

void GG_CloudStorage::setGetOptions(struct fb_esp_gcs_req_t *req, MB_String &header, bool hasParams)
{
    if (req->getOptions)
    {
        if (req->getOptions->generation.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_493;

            header += atoi(req->getOptions->generation.c_str());
        }
        if (req->getOptions->ifGenerationMatch.length() > 0)
        {
            if (hasParams)
            {

                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_494;

            header += atoi(req->getOptions->ifGenerationMatch.c_str());
        }

        if (req->getOptions->ifGenerationNotMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_495;

            header += atoi(req->getOptions->ifGenerationNotMatch.c_str());
        }

        if (req->getOptions->ifMetagenerationMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_496;

            header += atoi(req->getOptions->ifMetagenerationMatch.c_str());
        }

        if (req->getOptions->ifMetagenerationNotMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_497;

            header += atoi(req->getOptions->ifMetagenerationNotMatch.c_str());
        }

        if (req->getOptions->projection.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_498;
            header += req->getOptions->projection;
        }
    }
}

void GG_CloudStorage::setUploadOptions(struct fb_esp_gcs_req_t *req, MB_String &header, bool hasParams)
{
    if (req->uploadOptions)
    {
        if (req->uploadOptions->contentEncoding.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_499;
            header += req->uploadOptions->contentEncoding;
        }

        if (req->uploadOptions->ifGenerationMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_494;

            header += atoi(req->uploadOptions->ifGenerationMatch.c_str());
        }

        if (req->uploadOptions->ifGenerationNotMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_495;

            header += atoi(req->uploadOptions->ifGenerationNotMatch.c_str());
        }

        if (req->uploadOptions->ifMetagenerationMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_496;

            header += atoi(req->uploadOptions->ifMetagenerationMatch.c_str());
        }

        if (req->uploadOptions->ifMetagenerationNotMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_497;

            header += atoi(req->uploadOptions->ifMetagenerationNotMatch.c_str());
        }

        if (req->uploadOptions->kmsKeyName.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_500;
            header += req->uploadOptions->kmsKeyName;
        }

        if (req->uploadOptions->predefinedAcl.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_501;
            header += req->uploadOptions->predefinedAcl;
        }

        if (req->uploadOptions->projection.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_498;

            header += req->uploadOptions->projection;
        }
    }
}

void GG_CloudStorage::setRequestproperties(struct fb_esp_gcs_req_t *req, FirebaseJson *json, bool &hasProps)
{

    static FirebaseJson js;
    js.clear();

    if (req->requestProps)
    {
        if (req->requestProps->metadata.length() > 0)
        {
            hasProps = true;
            js.setJsonData(req->requestProps->metadata);
        }
    }

    js.add(pgm2Str(fb_esp_pgm_str_518), pgm2Str(fb_esp_pgm_str_519));
    json->add(pgm2Str(fb_esp_pgm_str_514), js);

    if (req->requestProps)
    {
        if (req->requestProps->acl.length() > 0)
        {
            static FirebaseJsonArray arr;
            arr.clear();
            arr.setJsonArrayData(req->requestProps->acl);
            json->add(pgm2Str(fb_esp_pgm_str_504), arr);
            hasProps = true;
        }
        if (req->requestProps->cacheControl.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_505), req->requestProps->cacheControl);
            hasProps = true;
        }
        if (req->requestProps->contentDisposition.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_506), req->requestProps->contentDisposition);
            hasProps = true;
        }
        if (req->requestProps->contentEncoding.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_507), req->requestProps->contentEncoding);
            hasProps = true;
        }
        if (req->requestProps->contentLanguage.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_508), req->requestProps->contentLanguage);
            hasProps = true;
        }
        if (req->requestProps->contentType.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_509), req->requestProps->contentType);
            hasProps = true;
        }
        if (req->requestProps->crc32c.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_510), req->requestProps->crc32c);
            hasProps = true;
        }
        if (req->requestProps->customTime.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_511), req->requestProps->customTime);
            hasProps = true;
        }

        if (req->requestProps->eventBasedHold.length() > 0)
        {
            if (strcmp(req->requestProps->eventBasedHold.c_str(), pgm2Str(fb_esp_pgm_str_107)))
                json->add(pgm2Str(fb_esp_pgm_str_512), true);
            else
                json->add(pgm2Str(fb_esp_pgm_str_512), false);

            hasProps = true;
        }

        if (req->requestProps->md5Hash.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_513), req->requestProps->md5Hash);
            hasProps = true;
        }

        if (req->requestProps->name.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_515), req->requestProps->name);
            hasProps = true;
        }
        if (req->requestProps->storageClass.length() > 0)
        {
            json->add(pgm2Str(fb_esp_pgm_str_516), req->requestProps->storageClass);
            hasProps = true;
        }

        if (req->requestProps->temporaryHold.length() > 0)
        {
            if (strcmp(req->requestProps->temporaryHold.c_str(), pgm2Str(fb_esp_pgm_str_107)))
                json->add(pgm2Str(fb_esp_pgm_str_517), true);
            else
                json->add(pgm2Str(fb_esp_pgm_str_517), false);
            hasProps = true;
        }
    }
}

void GG_CloudStorage::setDeleteOptions(struct fb_esp_gcs_req_t *req, MB_String &header, bool hasParams)
{
    if (req->deleteOptions)
    {
        if (req->deleteOptions->ifGenerationMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_494;

            header += atoi(req->deleteOptions->ifGenerationMatch.c_str());
        }

        if (req->deleteOptions->ifGenerationNotMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_495;

            header += atoi(req->deleteOptions->ifGenerationNotMatch.c_str());
        }

        if (req->deleteOptions->ifMetagenerationMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_496;

            header += atoi(req->deleteOptions->ifMetagenerationMatch.c_str());
        }

        if (req->deleteOptions->ifMetagenerationNotMatch.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_497;

            header += atoi(req->deleteOptions->ifMetagenerationNotMatch.c_str());
        }
    }
}

void GG_CloudStorage::setListOptions(struct fb_esp_gcs_req_t *req, MB_String &header, bool hasParams)
{

    if (req->listOptions)
    {
        if (req->listOptions->delimiter.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_485;
            header += req->listOptions->delimiter;
        }

        if (req->listOptions->endOffset.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_486;
            header += req->listOptions->endOffset;
        }

        if (req->listOptions->includeTrailingDelimiter.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_487;

            if (strcmp(req->listOptions->includeTrailingDelimiter.c_str(), pgm2Str(fb_esp_pgm_str_107)))
                header += fb_esp_pgm_str_107;
            else
                header += fb_esp_pgm_str_106;
        }

        if (req->listOptions->maxResults.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_484;

            header += atoi(req->listOptions->maxResults.c_str());
        }

        if (req->listOptions->pageToken.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_358;
            header += fb_esp_pgm_str_361;
            header += req->listOptions->pageToken;
        }

        if (req->listOptions->prefix.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_488;
            header += req->listOptions->prefix;
        }

        if (req->listOptions->projection.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_489;
            header += req->listOptions->projection;
        }

        if (req->listOptions->startOffset.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_490;
            header += req->listOptions->startOffset;
        }

        if (req->listOptions->versions.length() > 0)
        {
            if (hasParams)
            {
                header += fb_esp_pgm_str_172;
            }
            else
            {
                hasParams = true;
                header += fb_esp_pgm_str_173;
            }

            header += fb_esp_pgm_str_491;

            if (strcmp(req->listOptions->versions.c_str(), pgm2Str(fb_esp_pgm_str_107)))
                header += fb_esp_pgm_str_107;
            else
                header += fb_esp_pgm_str_106;
        }
    }
}

void GG_CloudStorage::sendUploadCallback(FirebaseData *fbdo, UploadStatusInfo &in, UploadProgressCallback cb, UploadStatusInfo *out)
{

    fbdo->session.gcs.cbUploadInfo.status = in.status;
    fbdo->session.gcs.cbUploadInfo.errorMsg = in.errorMsg;
    fbdo->session.gcs.cbUploadInfo.progress = in.progress;
    fbdo->session.gcs.cbUploadInfo.localFileName = in.localFileName;
    fbdo->session.gcs.cbUploadInfo.remoteFileName = in.remoteFileName;
    fbdo->session.gcs.cbUploadInfo.fileSize = in.fileSize;
    fbdo->session.gcs.cbUploadInfo.elapsedTime = in.elapsedTime;

    if (cb)
        cb(fbdo->session.gcs.cbUploadInfo);

    if (out)
    {
        out->errorMsg = fbdo->session.gcs.cbUploadInfo.errorMsg;
        out->status = fbdo->session.gcs.cbUploadInfo.status;
        out->progress = fbdo->session.gcs.cbUploadInfo.progress;
        out->localFileName = fbdo->session.gcs.cbUploadInfo.localFileName;
    }
}

void GG_CloudStorage::sendDownloadCallback(FirebaseData *fbdo, DownloadStatusInfo &in, DownloadProgressCallback cb, DownloadStatusInfo *out)
{

    fbdo->session.gcs.cbDownloadInfo.status = in.status;
    fbdo->session.gcs.cbDownloadInfo.errorMsg = in.errorMsg;
    fbdo->session.gcs.cbDownloadInfo.progress = in.progress;
    fbdo->session.gcs.cbDownloadInfo.localFileName = in.localFileName;
    fbdo->session.gcs.cbDownloadInfo.remoteFileName = in.remoteFileName;
    fbdo->session.gcs.cbDownloadInfo.fileSize = in.fileSize;
    fbdo->session.gcs.cbDownloadInfo.elapsedTime = in.elapsedTime;

    if (cb)
        cb(fbdo->session.gcs.cbDownloadInfo);

    if (out)
    {
        out->errorMsg = fbdo->session.gcs.cbDownloadInfo.errorMsg;
        out->status = fbdo->session.gcs.cbDownloadInfo.status;
        out->progress = fbdo->session.gcs.cbDownloadInfo.progress;
        out->localFileName = fbdo->session.gcs.cbDownloadInfo.localFileName;
    }
}

#if defined(ESP32)
void GG_CloudStorage::runResumableUploadTask(const char *taskName)
#elif defined(ESP8266) || defined(FB_ENABLE_EXTERNAL_CLIENT)
void GG_CloudStorage::runResumableUploadTask()
#endif
{
#if defined(ESP32)

    static GG_CloudStorage *_this = this;

    TaskFunction_t taskCode = [](void *param)
    {
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

        Signer.getCfg()->internal.resumable_upload_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName, 12000, NULL, 3, &Signer.getCfg()->internal.resumable_upload_task_handle, 1);

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

    char *pChunk = nullptr;
    char *temp = nullptr;
    char *header = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->session.resp_size;
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
    MB_String part1, part2, part3, part4, part5;
    size_t p1 = 0;
    size_t p2 = 0;

    MB_String tmp1, tmp2, tmp3, tmp4, tmp5;

    if (req->requestType == fb_esp_gcs_request_type_list)
    {
        tmp1 = fb_esp_pgm_str_476;
        tmp2 = fb_esp_pgm_str_477;
        tmp3 = fb_esp_pgm_str_482;
        tmp4 = fb_esp_pgm_str_483;
        tmp5 = fb_esp_pgm_str_3;
    }

    MB_String payload;
    fbdo->session.fcs.files.items.clear();

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

    if (!fbdo->tcpClient.connected())
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    if (req->requestType == fb_esp_gcs_request_type_download && strlen(ut->mbfs->name(mbfs_type req->storageType)) == 0)
    {

#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
        // Fix issue in ESP32 core v2.0.x filesystems
        // We can't open file (flash or sd) to write here because of truncated result, only append is success.
        // We have to remove existing file
        ut->mbfs->remove(req->localFileName, mbfs_type req->storageType);
#else
        int ret = ut->mbfs->open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_write);
        if (ret < 0)
        {
            fbdo->session.response.code = ret;
            return false;
        }
#endif
    }

    int availablePayload = chunkBufSize;

    dataTime = millis();

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
                        if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK || response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT)
                            fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
                        else
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
                            // parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            fbdo->session.http_code = response.httpCode;

                            fbdo->session.chunked_encoding = response.isChunkedEnc;

                            if (response.httpCode == 401)
                                Signer.authenticated = false;
                            else if (response.httpCode < 300)
                                Signer.authenticated = true;

                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK || response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT)
                                error.code = 0;

                            if (req->requestType == fb_esp_gcs_request_type_upload_resumable_init || req->requestType == fb_esp_gcs_request_type_upload_resumable_run)
                            {

                                if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT) // resume incomplete
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
                                            ruTask.req.storageType = req->storageType;
                                            ruTask.fbdo = fbdo;
                                            ruTask.req.requestType = fb_esp_gcs_request_type_upload_resumable_run;

                                            char *tmp6 = (char *)ut->newP(strlen(header));
                                            strncpy(tmp6, header + p1 + strlen_P(fb_esp_pgm_str_481), strlen(header) - p1 - strlen_P(fb_esp_pgm_str_481));
                                            ruTask.req.chunkRange = atoi(tmp6);
                                            ut->delP(&tmp6);

                                            ruTask.req.uploadCallback = req->uploadCallback;
                                            ruTask.req.uploadStatusInfo = req->uploadStatusInfo;

                                            _resumableUploadTasks.push_back(ruTask);

                                            fbdo->session.long_running_task++;
                                            _resumable_upload_task_enable = true;

                                            if (_resumableUploadTasks.size() == 1)
                                            {
#if defined(ESP32)
                                                runResumableUploadTask(pgm2Str(fb_esp_pgm_str_480));
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
                                    ruTask.req.storageType = req->storageType;
                                    ruTask.req.requestType = fb_esp_gcs_request_type_upload_resumable_run;

                                    ruTask.req.uploadCallback = req->uploadCallback;
                                    ruTask.req.uploadStatusInfo = req->uploadStatusInfo;

                                    _resumableUploadTasks.push_back(ruTask);

                                    fbdo->session.long_running_task++;
                                    _resumable_upload_task_enable = true;

                                    if (_resumableUploadTasks.size() == 1)
                                    {
#if defined(ESP32)
                                        runResumableUploadTask(pgm2Str(fb_esp_pgm_str_480));
#elif defined(ESP8266)
                                        runResumableUploadTask();
#endif
                                    }
                                }
                            }

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
                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.contentLen > 0 && (req->requestType == fb_esp_gcs_request_type_download || req->requestType == fb_esp_gcs_request_type_download_ota))
                            {

                                size_t available = fbdo->tcpClient.available();
                                dataTime = millis();

                                req->fileSize = response.contentLen;
                                error.code = 0;

                                DownloadStatusInfo in;
                                in.localFileName = req->localFileName;
                                in.remoteFileName = req->remoteFileName;
                                in.status = fb_esp_gcs_download_status_init;
                                in.fileSize = req->fileSize;
                                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);

                                if (fbdo->session.gcs.requestType == fb_esp_gcs_request_type_download_ota)
                                {
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
#if defined(ESP32)
                                    error.code = 0;
                                    if (!Update.begin(response.contentLen))
                                        error.code = FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE;
#elif defined(ESP8266)
                                    error.code = fbdo->tcpClient.beginUpdate(response.contentLen);

#endif
#endif
                                }

                                int bufLen = Signer.getCfg()->gcs.download_buffer_size;
                                if (bufLen < 512)
                                    bufLen = 512;

                                if (bufLen > 1024 * 16)
                                    bufLen = 1024 * 16;

                                uint8_t *buf = (uint8_t *)ut->newP(bufLen + 1, false);

                                while (fbdo->reconnect(dataTime) && fbdo->tcpClient.connected() && payloadRead < response.contentLen)
                                {
                                    if (available)
                                    {
                                        dataTime = millis();
                                        if ((int)available > bufLen)
                                            available = bufLen;

                                        size_t read = fbdo->tcpClient.readBytes(buf, available);
                                        if (read == available)
                                        {
                                            reportDownloadProgress(fbdo, req, payloadRead);

                                            if (fbdo->session.gcs.requestType == fb_esp_gcs_request_type_download_ota)
                                            {
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
                                                if (error.code == 0)
                                                {
                                                    if (Update.write(buf, read) != read)
                                                        error.code = FIREBASE_ERROR_FW_UPDATE_WRITE_FAILED;
                                                }
#endif
                                            }
                                            else
                                            {
                                                if (error.code == 0)
                                                {

#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
                                                    // We open file to append here
                                                    int ret = ut->mbfs->open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_append);

                                                    if (ret < 0)
                                                    {
                                                        fbdo->tcpClient.flush();
                                                        fbdo->session.response.code = ret;
                                                        return false;
                                                    }
#endif
                                                    if (ut->mbfs->write(mbfs_type req->storageType, buf, read) != (int)read)
                                                        error.code = MB_FS_ERROR_FILE_IO_ERROR;


#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
                                                    // We close file here after append
                                                    ut->mbfs->close(mbfs_type req->storageType);
#endif
                                                }
                                            }
                                        }

                                        payloadRead += available;
                                    }

                                    if (payloadRead == response.contentLen)
                                        break;

                                    available = fbdo->tcpClient.available();
                                }

                                reportDownloadProgress(fbdo, req, payloadRead);

                                ut->delP(&buf);
                                if (fbdo->session.gcs.requestType == fb_esp_gcs_request_type_download_ota)
                                {
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)

                                    if (error.code == 0)
                                    {
                                        if (!Update.end())
                                            error.code = FIREBASE_ERROR_FW_UPDATE_END_FAILED;
                                    }

#endif
                                }

                                if (error.code != 0)
                                    fbdo->session.response.code = error.code;

                                break;
                            }
                            else
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
                                    if (req->requestType == fb_esp_gcs_request_type_list)
                                    {
                                        delay(10);
                                        part1 = pChunk;
                                        p1 = part1.find(tmp1);
                                        if (p1 != MB_String::npos)
                                        {
                                            p2 = part1.find(tmp5, p1 + tmp1.length());
                                            if (p2 != MB_String::npos)
                                                part2 = part1.substr(p1 + tmp1.length(), p2 - p1 - tmp1.length());
                                        }

                                        if (part2.length() > 1)
                                        {
                                            if (part2[part2.length() - 1] != '/')
                                            {

                                                p1 = part1.find(tmp2);
                                                if (p1 != MB_String::npos)
                                                {
                                                    p2 = part1.find(tmp5, p1 + tmp2.length());
                                                    if (p2 != MB_String::npos)
                                                        part3 = part1.substr(p1 + tmp2.length(), p2 - p1 - tmp2.length());
                                                }

                                                p1 = part1.find(tmp3);
                                                if (p1 != MB_String::npos)
                                                {
                                                    p2 = part1.find(tmp5, p1 + tmp3.length());
                                                    if (p2 != MB_String::npos)
                                                        part4 = part1.substr(p1 + tmp3.length(), p2 - p1 - tmp3.length());
                                                }

                                                p1 = part1.find(tmp4);
                                                if (p1 != MB_String::npos)
                                                {
                                                    p2 = part1.find(tmp5, p1 + tmp4.length());
                                                    if (p2 != MB_String::npos)
                                                    {
                                                        part5 = part1.substr(p1 + tmp4.length(), p2 - p1 - tmp4.length());
                                                        fb_esp_fcs_file_list_item_t itm;
                                                        itm.name = part2;
                                                        itm.bucket = part3;
                                                        itm.contentType = part4;
                                                        itm.size = atoi(part5.c_str());
                                                        fbdo->session.fcs.files.items.push_back(itm);
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

                                ut->delP(&pChunk);

                                if (availablePayload < 0 || (payloadRead >= response.contentLen && !response.isChunkedEnc))
                                {
                                    fbdo->tcpClient.flush();
                                    break;
                                }
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
        if (payload.length() > 0)
        {
            if (payload[0] == '{')
            {
                if (!fbdo->session.jsonPtr)
                    fbdo->session.jsonPtr = new FirebaseJson();

                if (!fbdo->session.dataPtr)
                    fbdo->session.dataPtr = new FirebaseJsonData();

                fbdo->session.jsonPtr->setJsonData(payload.c_str());
                payload.clear();

                fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_257));
                if (fbdo->session.dataPtr->success)
                {
                    error.code = fbdo->session.dataPtr->to<int>();
                    fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_258));
                    if (fbdo->session.dataPtr->success)
                        fbdo->session.error = fbdo->session.dataPtr->to<const char *>();
                }
                else
                {
                    error.code = 0;

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_274))
                        fbdo->session.gcs.meta.name = fbdo->session.dataPtr->to<const char *>();

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_275))
                        fbdo->session.gcs.meta.bucket = fbdo->session.dataPtr->to<const char *>();

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_276))
                        fbdo->session.gcs.meta.generation = atoi(fbdo->session.dataPtr->to<const char *>());

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_503))
                        fbdo->session.gcs.meta.metageneration = atoi(fbdo->session.dataPtr->to<const char *>());

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_277))
                        fbdo->session.gcs.meta.contentType = fbdo->session.dataPtr->to<const char *>();

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_278))
                        fbdo->session.gcs.meta.size = atoi(fbdo->session.dataPtr->to<const char *>());

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_279))
                        fbdo->session.gcs.meta.etag = fbdo->session.dataPtr->to<const char *>();

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_478))
                        fbdo->session.gcs.meta.crc32 = fbdo->session.dataPtr->to<const char *>();

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_479))
                        fbdo->session.gcs.meta.downloadTokens = fbdo->session.dataPtr->to<const char *>();

                    if (parseJsonResponse(fbdo, fb_esp_pgm_str_492))
                        fbdo->session.gcs.meta.mediaLink = fbdo->session.dataPtr->to<const char *>();
                }

                fbdo->session.dataPtr->clear();
                fbdo->session.jsonPtr->clear();
            }
            fbdo->session.content_length = response.payloadLen;
        }

        return error.code == 0;
    }
    else
    {
        fbdo->tcpClient.flush();
    }

    return false;
}

bool GG_CloudStorage::parseJsonResponse(FirebaseData *fbdo, PGM_P key_path)
{
    fbdo->session.dataPtr->clear();
    fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(key_path));
    return fbdo->session.dataPtr->success;
}

#endif

#endif // ENABLE