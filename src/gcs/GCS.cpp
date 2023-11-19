
/**
 * Google's Cloud Storage class, GCS.cpp version 1.2.13
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
#include <Arduino.h>
#include "./mbfs/MB_MCU.h"
#include "./FirebaseFS.h"

#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)

#ifndef GOOGLE_CS_CPP
#define GOOGLE_CS_CPP

#include "GCS.h"

GG_CloudStorage::GG_CloudStorage()
{
}
GG_CloudStorage::~GG_CloudStorage()
{
}

bool GG_CloudStorage::mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr localFileName,
                              firebase_mem_storage_type storageType, firebase_gcs_upload_type uploadType,
                              MB_StringPtr remoteFileName, MB_StringPtr mime,
                              UploadOptions *uploadOptions, RequestProperties *requestProps,
                              UploadStatusInfo *status, UploadProgressCallback callback)
{
    struct firebase_gcs_req_t req;
    req.bucketID = bucketID;
    req.localFileName = localFileName;
    Core.ut.makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.mime = mime;
    req.uploadStatusInfo = status;
    req.uploadCallback = callback;
    req.uploadOptions = uploadOptions;
    req.requestProps = requestProps;
    if (status)
        req.uploadStatusInfo->localFileName = localFileName;

    if (uploadType == gcs_upload_type_simple)
        req.requestType = firebase_gcs_request_type_upload_simple;
    else if (uploadType == gcs_upload_type_multipart)
        req.requestType = firebase_gcs_request_type_upload_multipart;
    else if (uploadType == gcs_upload_type_resumable)
        req.requestType = firebase_gcs_request_type_upload_resumable_init;

    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::sendRequest(FirebaseData *fbdo, struct firebase_gcs_req_t *req)
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
    if (!fbdo->reconnect())
        return false;

    if (!Core.tokenReady())
        return false;

    if (fbdo->session.long_running_task > 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Core.internal.fb_processing)
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

    if (req->requestType == firebase_gcs_request_type_download || req->requestType == firebase_gcs_request_type_download_ota)
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

    if (req->requestType == firebase_gcs_request_type_upload_simple ||
        req->requestType == firebase_gcs_request_type_upload_multipart ||
        req->requestType == firebase_gcs_request_type_upload_resumable_init)
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

    Core.internal.fb_processing = true;

    bool ret = gcs_sendRequest(fbdo, req);

    Core.internal.fb_processing = false;

    if (!ret)
    {
        if (req->requestType == firebase_gcs_request_type_download || req->requestType == firebase_gcs_request_type_download_ota)
        {
            fbdo->session.gcs.cbDownloadInfo.status = firebase_gcs_download_status_error;
            DownloadStatusInfo in;
            makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_download_status_error,
                               0, 0, 0, fbdo->errorReason());
            sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
        }
        else if (req->requestType == firebase_gcs_request_type_upload_simple ||
                 req->requestType == firebase_gcs_request_type_upload_multipart ||
                 req->requestType == firebase_gcs_request_type_upload_resumable_init)
        {
            fbdo->session.gcs.cbUploadInfo.status = firebase_gcs_upload_status_error;
            UploadStatusInfo in;
            makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_upload_status_error,
                             0, 0, 0, fbdo->errorReason());
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        }

        fbdo->closeSession();
    }

    return ret;
}

bool GG_CloudStorage::mDownload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName,
                                MB_StringPtr localFileName, firebase_mem_storage_type storageType, StorageGetOptions *options,
                                DownloadProgressCallback callback)
{
    struct firebase_gcs_req_t req;
    req.bucketID = bucketID;
    req.localFileName = localFileName;
    Core.ut.makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.getOptions = options;
    req.downloadCallback = callback;
    req.requestType = firebase_gcs_request_type_download;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::mDownloadOTA(FirebaseData *fbdo, MB_StringPtr bucketID,
                                   MB_StringPtr remoteFileName, DownloadProgressCallback callback)
{
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))
    struct firebase_gcs_req_t req;
    req.bucketID = bucketID;
    req.remoteFileName = remoteFileName;
    req.requestType = firebase_gcs_request_type_download_ota;
    req.downloadCallback = callback;

    fbdo->closeSession();

    int rx_size = fbdo->session.bssl_rx_size;
    fbdo->session.bssl_rx_size = 16384;

    bool ret = sendRequest(fbdo, &req);

    fbdo->session.bssl_rx_size = rx_size;

    fbdo->closeSession();

    return ret;

#endif
    return false;
}

bool GG_CloudStorage::mDeleteFile(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr fileName, DeleteOptions *options)
{
    struct firebase_gcs_req_t req;
    req.requestType = firebase_gcs_request_type_delete;
    req.remoteFileName = fileName;
    req.bucketID = bucketID;
    req.deleteOptions = options;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::mListFiles(FirebaseData *fbdo, MB_StringPtr bucketID, ListOptions *options)
{
    struct firebase_gcs_req_t req;
    req.bucketID = bucketID;
    req.listOptions = options;
    req.requestType = firebase_gcs_request_type_list;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::mGetMetadata(FirebaseData *fbdo, MB_StringPtr bucketID,
                                   MB_StringPtr remoteFileName, StorageGetOptions *options)
{
    struct firebase_gcs_req_t req;
    req.bucketID = bucketID;
    req.remoteFileName = remoteFileName;
    req.getOptions = options;
    req.requestType = firebase_gcs_request_type_get_metadata;
    return sendRequest(fbdo, &req);
}

bool GG_CloudStorage::gcs_connect(FirebaseData *fbdo)
{
    MB_String host = firebase_pgm_str_31; // "googleapis.com"
    rescon(fbdo, host.c_str());
    fbdo->tcpClient.setSession(&fbdo->bsslSession);
    fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
    return true;
}

void GG_CloudStorage::rescon(FirebaseData *fbdo, const char *host)
{
    fbdo->_responseCallback = NULL;

    if (fbdo->session.cert_updated || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout ||
        fbdo->session.con_mode != firebase_con_mode_gc_storage || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->session.host = host;
    fbdo->session.con_mode = firebase_con_mode_gc_storage;
}

void GG_CloudStorage::reportUploadProgress(FirebaseData *fbdo, struct firebase_gcs_req_t *req, size_t readBytes)
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

        fbdo->session.gcs.cbUploadInfo.status = firebase_gcs_upload_status_upload;
        UploadStatusInfo in;
        makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_upload_status_upload,
                         p, 0, fbdo->tcpClient.dataTime, "");
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }
}

void GG_CloudStorage::reportDownloadProgress(FirebaseData *fbdo, struct firebase_gcs_req_t *req, size_t readBytes)
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

        fbdo->session.gcs.cbDownloadInfo.status = firebase_gcs_download_status_download;
        DownloadStatusInfo in;
        makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_download_status_download,
                           p, req->fileSize, fbdo->tcpClient.dataTime, "");
        sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
    }
}

bool GG_CloudStorage::gcs_sendRequest(FirebaseData *fbdo, struct firebase_gcs_req_t *req)
{

    fbdo->session.gcs.requestType = req->requestType;

    int ret = 0;

    if (req->requestType != firebase_gcs_request_type_download)
    {
        if (req->requestType == firebase_gcs_request_type_upload_simple ||
            req->requestType == firebase_gcs_request_type_upload_multipart ||
            req->requestType == firebase_gcs_request_type_upload_resumable_init)
        {

            ret = Core.mbfs.open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);
            // Fix in ESP32 core 2.0.x
            if (ret > 0)
                Core.mbfs.close(mbfs_type req->storageType);
            else if (ret < 0)
            {
                fbdo->session.response.code = ret;
                return false;
            }

            req->fileSize = ret;

            if (req->requestType == firebase_gcs_request_type_upload_resumable_init)
            {
                if (req->fileSize < gcs_min_chunkSize)
                    req->requestType = firebase_gcs_request_type_upload_multipart;
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
    MB_String boundary = Core.ut.getBoundary(&Core.mbfs, 15);

    fbdo->initJson();

    if (req->requestType == firebase_gcs_request_type_upload_simple ||
        req->requestType == firebase_gcs_request_type_upload_multipart ||
        req->requestType == firebase_gcs_request_type_upload_resumable_init)
    {
        UploadStatusInfo in;
        makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_upload_status_init,
                         0, req->fileSize, 0, "");
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }

    firebase_request_method method = http_undefined;

    if (req->requestType == firebase_gcs_request_type_upload_simple ||
        req->requestType == firebase_gcs_request_type_upload_multipart ||
        req->requestType == firebase_gcs_request_type_upload_resumable_init)
        method = http_post;
    else if (req->requestType == firebase_gcs_request_type_download ||
             req->requestType == firebase_gcs_request_type_download_ota ||
             req->requestType == firebase_gcs_request_type_list ||
             req->requestType == firebase_gcs_request_type_get_metadata)
        method = http_get;
    else if (req->requestType == firebase_gcs_request_type_delete)
        method = http_delete;
    else if (req->requestType == firebase_gcs_request_type_upload_resumable_run)
        method = http_put;

    if (method != http_undefined)
        Core.hh.addRequestHeaderFirst(header, method);

    if (req->requestType == firebase_gcs_request_type_upload_simple ||
        req->requestType == firebase_gcs_request_type_upload_multipart ||
        req->requestType == firebase_gcs_request_type_upload_resumable_init)
        header += firebase_gcs_pgm_str_2; // "/upload"

    if (req->requestType != firebase_gcs_request_type_upload_resumable_run)
    {
        header += firebase_gcs_pgm_str_1; // "/storage/v1/b/"
        header += req->bucketID;
        header += firebase_gcs_pgm_str_3; // "/o"
    }

    if (req->requestType == firebase_gcs_request_type_download ||
        req->requestType == firebase_gcs_request_type_download_ota)
    {
        header += firebase_pgm_str_1; // "/"
        header += Core.uh.encode(req->remoteFileName);
        header += firebase_gcs_pgm_str_4; // "?alt=media"
        setGetOptions(req, header, true);
        Core.hh.addRequestHeaderLast(header);
    }
    else if (req->requestType == firebase_gcs_request_type_upload_simple)
    {
        header += firebase_gcs_pgm_str_5; // "?uploadType=media&name="
        if (req->remoteFileName[0] == '/')
            header += Core.uh.encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += Core.uh.encode(req->remoteFileName);

        setUploadOptions(req, header, true);
        Core.hh.addRequestHeaderLast(header);
    }
    else if (req->requestType == firebase_gcs_request_type_upload_multipart)
    {
        header += firebase_gcs_pgm_str_6; // "?uploadType=multipart"
        setUploadOptions(req, header, true);
        Core.hh.addRequestHeaderLast(header);
    }
    else if (req->requestType == firebase_gcs_request_type_upload_resumable_init)
    {
        header += firebase_gcs_pgm_str_7; // "?uploadType=resumable&name="
        if (req->remoteFileName[0] == '/')
            header += Core.uh.encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += Core.uh.encode(req->remoteFileName);

        setUploadOptions(req, header, true);
        Core.hh.addRequestHeaderLast(header);
    }
    else if (req->requestType == firebase_gcs_request_type_upload_resumable_run)
    {
        firebase_url_info_t urlInfo;
        Core.uh.parse(&Core.mbfs, req->location, urlInfo);
        header += firebase_pgm_str_1; // "/"
        header += urlInfo.uri;
        Core.hh.addRequestHeaderLast(header);
        Core.hh.addHostHeader(header, urlInfo.host.c_str());
    }
    else if (req->requestType == firebase_gcs_request_type_delete)
    {
        header += firebase_pgm_str_1; // "/"
        if (req->remoteFileName[0] == '/')
            header += Core.uh.encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += Core.uh.encode(req->remoteFileName);

        setDeleteOptions(req, header, true);
        Core.hh.addRequestHeaderLast(header);
    }
    else if (req->requestType == firebase_gcs_request_type_list)
    {
        setListOptions(req, header, false);
        Core.hh.addRequestHeaderLast(header);
    }
    else if (req->requestType == firebase_gcs_request_type_get_metadata)
    {
        header += firebase_pgm_str_1; // "/"
        if (req->remoteFileName[0] == '/')
            header += Core.uh.encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += Core.uh.encode(req->remoteFileName);

        header += firebase_gcs_pgm_str_8; //"?alt=json"

        setGetOptions(req, header, true);
        Core.hh.addRequestHeaderLast(header);
    }

    if (req->requestType != firebase_gcs_request_type_upload_resumable_run)
    {
        Core.hh.addGAPIsHostHeader(header, firebase_pgm_str_61 /* "www" */);

        if (!Core.config->signer.test_mode)
        {
            Core.hh.addAuthHeaderFirst(header, Core.getTokenType());

            fbdo->tcpSend(header.c_str());
            header.clear();

            if (fbdo->session.response.code < 0)
                return false;

            fbdo->tcpSend(Core.getToken());

            if (fbdo->session.response.code < 0)
                return false;

            Core.hh.addNewLine(header);
        }
    }

    Core.hh.addUAHeader(header);
    // required for ESP32 core sdk v2.0.x.

    bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE)
    keepAlive = true;
#endif
    Core.hh.addConnectionHeader(header, keepAlive);

    if (req->requestType == firebase_gcs_request_type_upload_simple)
    {
        Core.hh.addContentTypeHeader(header, req->mime.c_str());
        Core.hh.addContentLengthHeader(header, req->fileSize);
    }
    else if (req->requestType == firebase_gcs_request_type_upload_multipart)
    {
        multipart_header += firebase_gcs_pgm_str_13; // "--"
        multipart_header += boundary;
        Core.hh.addNewLine(multipart_header);
        multipart_header += firebase_gcs_pgm_str_12; // "Content-Type: application/json; charset=UTF-8\r\n"
        Core.hh.addNewLine(multipart_header);

        Core.jh.clear(fbdo->session.jsonPtr);

        if (req->remoteFileName[0] == '/')
            Core.jh.addString(fbdo->session.jsonPtr, firebase_pgm_str_66 /* "name" */,
                              Core.uh.encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1)));
        else
            Core.jh.addString(fbdo->session.jsonPtr, firebase_pgm_str_66 /* "name" */,
                              Core.uh.encode(req->remoteFileName));

        Core.jh.addString(fbdo->session.jsonPtr, firebase_storage_ss_pgm_str_9 /* "contentType" */, req->mime);

        bool hasProps = false;
        setRequestproperties(req, fbdo->session.jsonPtr, hasProps);

        multipart_header += fbdo->session.jsonPtr->raw();
        Core.hh.addNewLine(multipart_header);
        Core.hh.addNewLine(multipart_header);

        multipart_header += firebase_gcs_pgm_str_13; // "--"
        multipart_header += boundary;
        Core.hh.addNewLine(multipart_header);
        Core.hh.addNewLine(multipart_header);

        Core.hh.addNewLine(multipart_header2);
        multipart_header2 += firebase_gcs_pgm_str_13; // "--"
        multipart_header2 += boundary;
        multipart_header2 += firebase_gcs_pgm_str_13; // "--"

        header += firebase_pgm_str_33;     // "Content-Type: "
        header += firebase_gcs_pgm_str_14; // "multipart/related; boundary="
        header += boundary;
        Core.hh.addNewLine(header);

        Core.hh.addContentLengthHeader(header, req->fileSize + multipart_header.length() + multipart_header2.length());
    }
    else if (req->requestType == firebase_gcs_request_type_upload_resumable_init)
    {
        header += firebase_gcs_pgm_str_9; //"X-Upload-Content-Type: "
        header += req->mime;
        Core.hh.addNewLine(header);

        header += firebase_gcs_pgm_str_10; // "X-Upload-Content-Length: "
        header += req->fileSize;
        Core.hh.addNewLine(header);

        header += firebase_gcs_pgm_str_12; // "Content-Type: application/json; charset=UTF-8\r\n"

        Core.jh.clear(fbdo->session.jsonPtr);

        bool hasProps = false;
        setRequestproperties(req, fbdo->session.jsonPtr, hasProps);

        Core.hh.addContentLengthHeader(header, strlen(fbdo->session.jsonPtr->raw()));
    }
    else if (req->requestType == firebase_gcs_request_type_upload_resumable_run)
    {

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

        Core.hh.addContentLengthHeader(header, req->chunkLen);

        if (req->chunkRange != -1 || req->location.length() > 0)
        {
            header += firebase_gcs_pgm_str_11; // "Content-Range: bytes "
            header += req->chunkPos;
            header += firebase_pgm_str_14; // "-"
            header += req->chunkPos + req->chunkLen - 1;
            header += firebase_pgm_str_1; // "/"
            header += req->fileSize;
            Core.hh.addNewLine(header);
        }
    }

    Core.hh.addNewLine(header);

    if (req->requestType == firebase_gcs_request_type_download ||
        req->requestType == firebase_gcs_request_type_download_ota)
    {
        fbdo->tcpSend(header.c_str());
        header.clear();
        if (fbdo->session.response.code < 0)
            return false;
    }
    else
    {
        fbdo->tcpSend(header.c_str());
        header.clear();
        if (fbdo->session.response.code < 0)
            return false;

        if (req->requestType == firebase_gcs_request_type_upload_resumable_init)
        {
            if (fbdo->session.jsonPtr)
                fbdo->tcpSend(fbdo->session.jsonPtr->raw());

            if (fbdo->session.response.code < 0)
                return false;
        }
    }

    boundary.clear();

    if (fbdo->session.response.code > 0)
    {

        fbdo->session.gcs.storage_type = req->storageType;
        if (req->requestType == firebase_gcs_request_type_upload_simple ||
            req->requestType == firebase_gcs_request_type_upload_multipart)
        {
            fbdo->session.long_running_task++;

            if (req->requestType == firebase_gcs_request_type_upload_multipart)
            {
                fbdo->tcpSend(multipart_header.c_str());
                multipart_header.clear();

                if (fbdo->session.response.code < 0)
                    return false;
            }

            int available = req->fileSize;
            size_t byteRead = 0;
            int bufLen = Core.ut.getUploadBufSize(Core.config, firebase_con_mode_gc_storage);
            uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen + 1, false));
            int read = 0;
            // Fix in ESP32 core 2.0.x
            Core.mbfs.open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                read = Core.mbfs.read(mbfs_type req->storageType, buf, available);

                byteRead += read;

                reportUploadProgress(fbdo, req, byteRead);

                if ((int)fbdo->tcpWrite(buf, read) != read)
                {
                    fbdo->session.response.code = FIREBASE_ERROR_UPLOAD_DATA_ERRROR;
                    fbdo->closeSession();
                    break;
                }

                available = Core.mbfs.available(mbfs_type req->storageType);
            }

            Core.mbfs.delP(&buf);
            fbdo->session.long_running_task--;

            Core.mbfs.close(mbfs_type req->storageType);

            if (req->requestType == firebase_gcs_request_type_upload_multipart)
            {
                fbdo->tcpSend(multipart_header2.c_str());
                multipart_header2.clear();

                if (fbdo->session.response.code < 0)
                    return false;
            }

            reportUploadProgress(fbdo, req, req->fileSize);
        }
        else if (req->requestType == firebase_gcs_request_type_upload_resumable_run)
        {
            size_t byteRead = 0;
            int available = 0;

            int bufLen = Core.ut.getUploadBufSize(Core.config, firebase_con_mode_gc_storage);
            uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen + 1));
            int read = 0;
            size_t totalBytes = req->fileSize;

            if (req->chunkRange != -1 || req->location.length() > 0)
            {
                // Fix in ESP32 core 2.0.x
                Core.mbfs.close(mbfs_type req->storageType);
                Core.mbfs.open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);
                byteRead = req->chunkPos;
                Core.mbfs.seek(mbfs_type req->storageType, req->chunkPos);
                totalBytes = req->chunkPos + req->chunkLen;
            }

            available = totalBytes;

            reportUploadProgress(fbdo, req, req->chunkPos);

            while (byteRead < totalBytes)
            {
                FBUtils::idle();
                if (available > bufLen)
                    available = bufLen;
                read = Core.mbfs.read(mbfs_type req->storageType, buf, available);

                if (fbdo->tcpClient.connected())
                {
                    if ((int)fbdo->tcpWrite(buf, read) != read)
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

                available = Core.mbfs.available(mbfs_type req->storageType);
                if (byteRead + available > totalBytes)
                    available = totalBytes - byteRead;
            }

            Core.mbfs.delP(&buf);

            if (Core.mbfs.available(mbfs_type req->storageType) == 0)
                Core.mbfs.close(mbfs_type req->storageType);
        }

        if (fbdo->tcpClient.connected())
        {
            bool ret = handleResponse(fbdo, req);
            fbdo->closeSession();
            if (ret)
            {
                if (Core.mbfs.ready(mbfs_type req->storageType) && req->requestType == firebase_gcs_request_type_download)
                    Core.mbfs.close(mbfs_type req->storageType);

                Core.internal.fb_processing = false;
                if (req->requestType == firebase_gcs_request_type_download ||
                    req->requestType == firebase_gcs_request_type_download_ota)
                {
                    DownloadStatusInfo in;
                    makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_download_status_complete,
                                       100, req->fileSize, 0, "");
                    sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
                }
                else if (req->requestType == firebase_gcs_request_type_upload_simple ||
                         req->requestType == firebase_gcs_request_type_upload_multipart)
                {
                    UploadStatusInfo in;
                    makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_upload_status_complete,
                                     100, 0, 0, "");
                    sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
                }
                else if (req->requestType == firebase_gcs_request_type_upload_resumable_run)
                {
                    if (Core.mbfs.available(mbfs_type req->storageType) == 0)
                    {
                        UploadStatusInfo in;
                        makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_upload_status_complete,
                                         100, 0, 0, "");
                        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
                        fbdo->session.long_running_task -= _resumableUploadTasks.size();
                        _resumableUploadTasks.clear();
                    }
                }
                return true;
            }
        }
    }

    if (req->requestType == firebase_gcs_request_type_download || req->requestType == firebase_gcs_request_type_download_ota)
    {
        DownloadStatusInfo in;
        makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_download_status_error,
                           0, 0, 0, fbdo->errorReason());
        sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
    }
    else if (req->requestType == firebase_gcs_request_type_upload_resumable_init ||
             req->requestType == firebase_gcs_request_type_upload_resumable_run ||
             req->requestType == firebase_gcs_request_type_upload_simple ||
             req->requestType == firebase_gcs_request_type_upload_multipart)
    {
        UploadStatusInfo in;
        makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_upload_status_error,
                         0, 0, 0, fbdo->errorReason());
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        fbdo->session.long_running_task -= _resumableUploadTasks.size();
        _resumableUploadTasks.clear();
    }

    if (Core.mbfs.ready(mbfs_type req->storageType) && req->requestType == firebase_gcs_request_type_download)
        Core.mbfs.close(mbfs_type req->storageType);

    Core.internal.fb_processing = false;

    return false;
}

void GG_CloudStorage::setGetOptions(struct firebase_gcs_req_t *req, MB_String &header, bool hasParams)
{
    if (req->getOptions)
    {
        Core.uh.addParam(header, firebase_gcs_pgm_str_15 /* "generation=" */,
                         req->getOptions->generation, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_16 /* "ifGenerationMatch=" */,
                         req->getOptions->ifGenerationMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_17 /* "ifGenerationNotMatch=" */,
                         req->getOptions->ifGenerationNotMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_18 /* "ifMetagenerationMatch=" */,
                         req->getOptions->ifMetagenerationMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_19 /* "ifMetagenerationNotMatch=" */,
                         req->getOptions->ifMetagenerationNotMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_21 /* "projection=" */,
                         req->getOptions->projection, hasParams);
    }
}

void GG_CloudStorage::setUploadOptions(struct firebase_gcs_req_t *req, MB_String &header, bool hasParams)
{
    if (req->uploadOptions)
    {
        Core.uh.addParam(header, firebase_gcs_pgm_str_20 /* "contentEncoding=" */,
                         req->uploadOptions->contentEncoding, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_16 /* "ifGenerationMatch=" */,
                         req->uploadOptions->ifGenerationMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_17 /* "ifGenerationNotMatch=" */,
                         req->uploadOptions->ifGenerationNotMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_18 /* "ifMetagenerationMatch=" */,
                         req->uploadOptions->ifMetagenerationMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_19 /* "ifMetagenerationNotMatch=" */,
                         req->uploadOptions->ifMetagenerationNotMatch, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_22 /* "kmsKeyName=" */,
                         req->uploadOptions->kmsKeyName, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_23 /* "predefinedAcl=" */,
                         req->uploadOptions->predefinedAcl, hasParams);
        Core.uh.addParam(header, firebase_gcs_pgm_str_21 /* "projection=" */,
                         req->uploadOptions->projection, hasParams);
    }
}

void GG_CloudStorage::setRequestproperties(struct firebase_gcs_req_t *req, FirebaseJson *json, bool &hasProps)
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

    Core.jh.addString(json, firebase_gcs_pgm_str_24 /* "firebaseStorageDownloadTokens" */,
                      MB_String(firebase_gcs_pgm_str_25 /* "a82781ce-a115-442f-bac6-a52f7f63b3e8" */));
    Core.jh.addObject(json, firebase_gcs_pgm_str_36 /* "metadata" */, &js, false);

    if (req->requestProps)
    {
        Core.jh.addArrayString(json, firebase_gcs_pgm_str_26 /* "acl" */,
                               req->requestProps->acl, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_27 /* "cacheControl" */),
                          req->requestProps->cacheControl, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_28 /* "contentDisposition" */),
                          req->requestProps->contentDisposition, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_29 /* "contentEncoding" */),
                          req->requestProps->contentEncoding, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_30 /* "contentLanguage" */),
                          req->requestProps->contentLanguage, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_31 /* "contentType" */),
                          req->requestProps->contentType, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_32 /* "crc32c" */),
                          req->requestProps->crc32c, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_33 /* "customTime" */),
                          req->requestProps->customTime, hasProps);
        Core.jh.addBoolString(json, pgm2Str(firebase_gcs_pgm_str_34 /* "eventBasedHold" */),
                              req->requestProps->eventBasedHold, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_35 /* "md5Hash" */),
                          req->requestProps->md5Hash, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_37 /* "name" */),
                          req->requestProps->name, hasProps);
        Core.jh.addString(json, pgm2Str(firebase_gcs_pgm_str_38 /* "storageClass" */),
                          req->requestProps->storageClass, hasProps);
        Core.jh.addBoolString(json, pgm2Str(firebase_gcs_pgm_str_39 /* "temporaryHold" */),
                              req->requestProps->temporaryHold, hasProps);
    }
}

void GG_CloudStorage::setDeleteOptions(struct firebase_gcs_req_t *req, MB_String &header, bool hasParams)
{
    if (req->deleteOptions)
    {
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_16 /* "ifGenerationMatch=" */,
                                   req->deleteOptions->ifGenerationMatch, hasParams)
                      ? Core.sh.intStr2Str(req->deleteOptions->ifGenerationMatch)
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_17 /* "ifGenerationNotMatch=" */,
                                   req->deleteOptions->ifGenerationNotMatch, hasParams)
                      ? Core.sh.intStr2Str(req->deleteOptions->ifGenerationNotMatch)
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_18 /* "fMetagenerationMatch=" */,
                                   req->deleteOptions->ifMetagenerationMatch, hasParams)
                      ? Core.sh.intStr2Str(req->deleteOptions->ifMetagenerationMatch)
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_19 /* "ifMetagenerationNotMatch=" */,
                                   req->deleteOptions->ifMetagenerationNotMatch, hasParams)
                      ? Core.sh.intStr2Str(req->deleteOptions->ifMetagenerationNotMatch)
                      : "";
    }
}

void GG_CloudStorage::setListOptions(struct firebase_gcs_req_t *req, MB_String &header, bool hasParams)
{
    if (req->listOptions)
    {
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_41 /* "delimiter=" */,
                                   req->listOptions->delimiter, hasParams)
                      ? req->listOptions->delimiter
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_42 /* "endOffset=" */,
                                   req->listOptions->endOffset, hasParams)
                      ? req->listOptions->endOffset
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_43 /* "includeTrailingDelimiter=" */,
                                   req->listOptions->includeTrailingDelimiter, hasParams)
                      ? Core.sh.boolStr2Str(req->listOptions->includeTrailingDelimiter)
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_40 /* "maxResults=" */,
                                   req->listOptions->maxResults, hasParams)
                      ? Core.sh.intStr2Str(req->listOptions->maxResults)
                      : "";
        header += Core.uh.addParam(header, firebase_pgm_str_65 /* "pageToken" */,
                                   req->listOptions->pageToken, hasParams)
                      ? req->listOptions->pageToken
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_44 /* "prefix=" */,
                                   req->listOptions->prefix, hasParams)
                      ? req->listOptions->prefix
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_21 /* "projection=" */,
                                   req->listOptions->projection, hasParams)
                      ? req->listOptions->projection
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_45 /* "startOffset=" */,
                                   req->listOptions->startOffset, hasParams)
                      ? req->listOptions->startOffset
                      : "";
        header += Core.uh.addParam(header, firebase_gcs_pgm_str_46 /* "versions=" */,
                                   req->listOptions->versions, hasParams)
                      ? Core.sh.boolStr2Str(req->listOptions->versions)
                      : "";
    }
}

void GG_CloudStorage::sendUploadCallback(FirebaseData *fbdo, UploadStatusInfo &in,
                                         UploadProgressCallback cb, UploadStatusInfo *out)
{
    fbdo->session.gcs.cbUploadInfo = in;

    if (cb)
        cb(fbdo->session.gcs.cbUploadInfo);

    if (out)
        out = &fbdo->session.gcs.cbUploadInfo;
}

void GG_CloudStorage::sendDownloadCallback(FirebaseData *fbdo, DownloadStatusInfo &in,
                                           DownloadProgressCallback cb, DownloadStatusInfo *out)
{

    fbdo->session.gcs.cbDownloadInfo = in;

    if (cb)
        cb(fbdo->session.gcs.cbDownloadInfo);

    if (out)
        out = &fbdo->session.gcs.cbDownloadInfo;
}

void GG_CloudStorage::makeUploadStatus(UploadStatusInfo &info, const MB_String &local,
                                       const MB_String &remote, firebase_gcs_upload_status status, size_t progress,
                                       size_t fileSize, int elapsedTime, const MB_String &msg)
{
    info.localFileName = local;
    info.remoteFileName = remote;
    info.status = status;
    info.fileSize = fileSize;
    info.progress = progress;
    info.elapsedTime = elapsedTime;
    info.errorMsg = msg;
}
void GG_CloudStorage::makeDownloadStatus(DownloadStatusInfo &info, const MB_String &local,
                                         const MB_String &remote, firebase_gcs_download_status status,
                                         size_t progress, size_t fileSize,
                                         int elapsedTime, const MB_String &msg)
{
    info.localFileName = local;
    info.remoteFileName = remote;
    info.status = status;
    info.fileSize = fileSize;
    info.progress = progress;
    info.elapsedTime = elapsedTime;
    info.errorMsg = msg;
}

void GG_CloudStorage::mRunResumableUploadTask()
{
    if (Core.internal.resumable_upload_loop_task_enabnle)
    {
#if defined(ESP32)

        static GG_CloudStorage *_this = this;
        MB_String taskName = "ResumableUpload_";
        taskName += random(1,100);

        TaskFunction_t taskCode = [](void *param)
        {
            while (_this->_resumable_upload_task_enable)
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);

                if (!_this->mRunResumableUpload())
                    break;

                yield();

                if (!Core.internal.resumable_upload_loop_task_enabnle)
                    break;
            }

            Core.internal.resumable_upload_task_handle = NULL;
            vTaskDelete(NULL);
        };
        xTaskCreatePinnedToCore(taskCode, taskName.c_str(), 12000, NULL, 3, &Core.internal.resumable_upload_task_handle, 1);

#elif defined(ESP8266)

        if (!mRunResumableUpload())
            return;

        if (_resumableUploadTasks.size() > 0)
            Core.set_scheduled_callback(std::bind(&GG_CloudStorage::mRunResumableUploadTask, this));

#endif
    }
    else
        mRunResumableUpload();
}

bool GG_CloudStorage::mRunResumableUpload()
{
    if (_resumableUploadTasks.size() == 0)
        return false;

    if (_resumableUplaodTaskIndex < _resumableUploadTasks.size() - 1)
        _resumableUplaodTaskIndex++;
    else
        _resumableUplaodTaskIndex = 0;

    struct fb_gcs_upload_resumable_task_info_t *taskInfo = &_resumableUploadTasks[_resumableUplaodTaskIndex];

    if (!taskInfo->done)
    {
        taskInfo->done = true;
        if (!gcs_sendRequest(taskInfo->fbdo, &taskInfo->req))
            taskInfo->fbdo->closeSession();
    }

    mResumableUploadUpdate();

    return _resumableUploadTasks.size() > 0;
}

void GG_CloudStorage::mResumableUploadUpdate()
{
    size_t n = 0;
    for (size_t i = 0; i < _resumableUploadTasks.size(); i++)
        if (_resumableUploadTasks[i].done)
            n++;

    if (n == _resumableUploadTasks.size())
    {
        _resumableUploadTasks.clear();
        _resumable_upload_task_enable = false;
    }
}

void GG_CloudStorage::runResumableUploadTask()
{
    Core.internal.resumable_upload_loop_task_enabnle = false;

#if defined(ESP32)
    if (Core.internal.resumable_upload_task_handle)
        return;
#endif

    if (_resumableUploadTasks.size() > 0)
        mRunResumableUploadTask();
}

bool GG_CloudStorage::handleResponse(FirebaseData *fbdo, struct firebase_gcs_req_t *req)
{

    if (!fbdo->reconnect())
        return false;

    bool isOTA = fbdo->session.gcs.requestType == firebase_gcs_request_type_download_ota;

    MB_String payload;

    struct server_response_data_t response;
    struct firebase_tcp_response_handler_t tcpHandler;

    Core.hh.initTCPSession(fbdo->session);
    Core.hh.intTCPHandler(&fbdo->tcpClient, tcpHandler, 2048, fbdo->session.resp_size, &payload, isOTA);

    firebase_fcs_file_list_item_t itm;
    int fileInfoStage = 0;
    fbdo->session.fcs.files.items.clear();

    bool isList = false, isMeta = false;
    int upos = 0;

    if (!fbdo->waitResponse(tcpHandler))
        return false;

    if (!fbdo->tcpClient.connected())
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    if (req->requestType == firebase_gcs_request_type_download &&
        strlen(Core.mbfs.name(mbfs_type req->storageType)) == 0 &&
        !fbdo->prepareDownload(req->localFileName, req->storageType, true))
        return false;

    bool ruTask = req->requestType == firebase_gcs_request_type_upload_resumable_init ||
                  req->requestType == firebase_gcs_request_type_upload_resumable_run;

    bool complete = false;

    while (tcpHandler.available() > 0 /* data available to read payload */ ||
           tcpHandler.payloadRead < response.contentLen /* incomplete content read  */)
    {
        if (!fbdo->readResponse(nullptr, tcpHandler, response) && !ruTask)
            break;

        // handle resumable upload headers
        if (tcpHandler.headerEnded && ruTask)
        {
            // reset resumable upload task flag
            ruTask = false;

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK ||
                response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT ||
                response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT)
                fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
            else
                fbdo->session.response.code = response.httpCode;

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT) // resume incomplete
            {
                int p1 = 0;
                if (Core.sh.find(tcpHandler.header, firebase_gcs_pgm_str_48 /* "Range: bytes=0-" */, false, 0, p1))
                {
                    if (_resumableUploadTasks.size() > 0)
                    {
                        struct fb_gcs_upload_resumable_task_info_t ruTask;
                        fbdo->createResumableTask(ruTask, req->fileSize, req->location,
                                                  req->localFileName, req->remoteFileName, req->storageType,
                                                  firebase_gcs_request_type_upload_resumable_run);
                        ruTask.req.chunkRange = atoi(tcpHandler.header.substr(p1 + strlen_P(firebase_gcs_pgm_str_48 /* "Range: bytes=0-" */),
                                                                              tcpHandler.header.length() - p1 - strlen_P(firebase_gcs_pgm_str_48 /* "Range: bytes=0-" */))
                                                         .c_str());
                        ruTask.req.uploadCallback = req->uploadCallback;
                        ruTask.req.uploadStatusInfo = req->uploadStatusInfo;
                        _resumableUploadTasks.push_back(ruTask);

                        fbdo->session.long_running_task++;
                        _resumable_upload_task_enable = true;
                    }
                }
            }
            else if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.location.length() > 0)
            {
                struct fb_gcs_upload_resumable_task_info_t ruTask;
                fbdo->createResumableTask(ruTask, req->fileSize, response.location, req->localFileName,
                                          req->remoteFileName, req->storageType, firebase_gcs_request_type_upload_resumable_run);
                ruTask.req.uploadCallback = req->uploadCallback;
                ruTask.req.uploadStatusInfo = req->uploadStatusInfo;
                _resumableUploadTasks.push_back(ruTask);

                fbdo->session.long_running_task++;
                _resumable_upload_task_enable = true;
            }

            if (_resumable_upload_task_enable && _resumableUploadTasks.size() == 1)
                mRunResumableUploadTask();

            if (_resumable_upload_task_enable && response.contentLen == 0)
                break;
        }

        // handle payload
        if (tcpHandler.pChunkIdx > 0)
        {
            tcpHandler.header.clear();

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.contentLen > 0 &&
                (fbdo->session.gcs.requestType == firebase_gcs_request_type_download ||
                 fbdo->session.gcs.requestType == firebase_gcs_request_type_download_ota))
            {
                tcpHandler.dataTime = millis();

                req->fileSize = response.contentLen;
                tcpHandler.error.code = 0;

                DownloadStatusInfo in;
                makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_gcs_download_status_init,
                                   0, req->fileSize, 0, "");
                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);

                int bufLen = Core.config->gcs.download_buffer_size;
                if (bufLen < 512)
                    bufLen = 512;

                if (bufLen > 1024 * 16)
                    bufLen = 1024 * 16;
                uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen, false));
                int stage = 0;

                if (isOTA)
                    fbdo->prepareDownloadOTA(tcpHandler, response);

                while (fbdo->processDownload(req->localFileName, req->storageType, buf, bufLen, tcpHandler,
                                             response, stage, isOTA))
                {
                    if (stage)
                        reportDownloadProgress(fbdo, req, tcpHandler.payloadRead);
                }

                Core.mbfs.delP(&buf);

                reportDownloadProgress(fbdo, req, tcpHandler.payloadRead);

                if (isOTA)
                    fbdo->endDownloadOTA(tcpHandler);

                if (tcpHandler.error.code != 0)
                    fbdo->session.response.code = tcpHandler.error.code;

                break;
            }
            else
            {

                MB_String pChunk;
                fbdo->readPayload(&pChunk, tcpHandler, response);

                // Last chunk?
                if (Core.ut.isChunkComplete(&tcpHandler, &response, complete))
                    goto skip;

                if (pChunk.length() > 0)
                {
                    if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK)
                    {
                        FBUtils::idle();

                        isList = fbdo->session.gcs.requestType == firebase_gcs_request_type_list;
                        isMeta = fbdo->session.gcs.requestType != firebase_gcs_request_type_download_ota &&
                                 fbdo->session.gcs.requestType != firebase_gcs_request_type_download;

                        if (fbdo->getUploadInfo(1, fileInfoStage, pChunk, isList, isMeta, &itm, upos))
                            fbdo->session.fcs.files.items.push_back(itm);
                    }
                    payload += pChunk;
                }
            }
        }
    }

skip:

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    fbdo->getAllUploadInfo(1, fileInfoStage, payload, isList, isMeta, &itm);

    // parse the payload for error
    fbdo->getError(payload, tcpHandler, response, true);

    return tcpHandler.error.code == 0;
}

bool GG_CloudStorage::parseJsonResponse(FirebaseData *fbdo, PGM_P key_path)
{
    return Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, key_path);
}

#endif

#endif // ENABLE