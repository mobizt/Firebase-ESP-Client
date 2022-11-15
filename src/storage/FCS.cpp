/**
 * Google's Firebase Storage class, FCS.cpp version 1.2.2
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 15, 2022
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

#ifdef ENABLE_FB_STORAGE

#ifndef FB_Storage_CPP
#define FB_Storage_CPP

#include "FCS.h"

FB_Storage::FB_Storage()
{
}
FB_Storage ::~FB_Storage()
{
}

void FB_Storage::begin(UtilsClass *u)
{
    ut = u;
}

bool FB_Storage::sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req)
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

    fcs_connect(fbdo);
    fbdo->session.fcs.meta.name.clear();
    fbdo->session.fcs.meta.bucket.clear();
    fbdo->session.fcs.meta.contentType.clear();
    fbdo->session.fcs.meta.crc32.clear();
    fbdo->session.fcs.meta.etag.clear();
    fbdo->session.fcs.meta.downloadTokens.clear();
    fbdo->session.fcs.meta.generation = 0;
    fbdo->session.fcs.meta.size = 0;

    if (req->requestType == fb_esp_fcs_request_type_download || req->requestType == fb_esp_fcs_request_type_download_ota || req->requestType == fb_esp_fcs_request_type_upload)
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

    Signer.getCfg()->internal.fb_processing = true;

    bool ret = fcs_sendRequest(fbdo, req);

    Signer.getCfg()->internal.fb_processing = false;

    if (!ret)
    {
        if (req->requestType == fb_esp_fcs_request_type_download || req->requestType == fb_esp_fcs_request_type_download_ota)
        {
            fbdo->session.fcs.cbDownloadInfo.status = fb_esp_fcs_download_status_error;
            FCS_DownloadStatusInfo in;
            in.localFileName = req->localFileName;
            in.remoteFileName = req->remoteFileName;
            in.status = fb_esp_fcs_download_status_error;
            in.progress = 0;
            in.errorMsg = fbdo->errorReason().c_str();
            sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
        }
        else if (req->requestType == fb_esp_fcs_request_type_upload || req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
        {
            fbdo->session.fcs.cbUploadInfo.status = fb_esp_fcs_upload_status_error;
            FCS_UploadStatusInfo in;
            in.localFileName = req->localFileName;
            in.remoteFileName = req->remoteFileName;
            in.status = fb_esp_fcs_upload_status_error;
            in.progress = 0;
            in.errorMsg = fbdo->errorReason().c_str();
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        }
    }

    return ret;
}

bool FB_Storage::mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr localFileName, fb_esp_mem_storage_type storageType, MB_StringPtr remoteFileName, MB_StringPtr mime, FCS_UploadProgressCallback callback)
{
    struct fb_esp_fcs_req_t req;
    req.localFileName = localFileName;
    ut->makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.requestType = fb_esp_fcs_request_type_upload;
    req.bucketID = bucketID;
    req.mime = mime;
    req.uploadCallback = callback;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, const uint8_t *data, size_t len, MB_StringPtr remoteFileName, MB_StringPtr mime, FCS_UploadProgressCallback callback)
{
    struct fb_esp_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.requestType = fb_esp_fcs_request_type_upload_pgm_data;
    req.bucketID = bucketID;
    req.mime = mime;
    req.pgmArc = data;
    req.pgmArcLen = len;
    req.uploadCallback = callback;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mDownload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName, MB_StringPtr localFileName, fb_esp_mem_storage_type storageType, FCS_DownloadProgressCallback callback)
{
    struct fb_esp_fcs_req_t req;
    req.localFileName = localFileName;
    ut->makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.requestType = fb_esp_fcs_request_type_download;
    req.bucketID = bucketID;
    req.downloadCallback = callback;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mDownloadOTA(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName, FCS_DownloadProgressCallback callback)
{
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
    struct fb_esp_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.requestType = fb_esp_fcs_request_type_download_ota;
    req.bucketID = bucketID;
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

bool FB_Storage::mGetMetadata(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName)
{
    struct fb_esp_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;
    req.requestType = fb_esp_fcs_request_type_get_meta;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mDeleteFile(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName)
{
    struct fb_esp_fcs_req_t req;
    req.requestType = fb_esp_fcs_request_type_delete;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;

    return sendRequest(fbdo, &req);
}

bool FB_Storage::mListFiles(FirebaseData *fbdo, MB_StringPtr bucketID)
{
    struct fb_esp_fcs_req_t req;
    req.bucketID = bucketID;
    req.requestType = fb_esp_fcs_request_type_list;

    return sendRequest(fbdo, &req);
}

void FB_Storage::rescon(FirebaseData *fbdo, const char *host)
{
    if (fbdo->session.cert_updated || !fbdo->session.connected || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout || fbdo->session.con_mode != fb_esp_con_mode_storage || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->session.host = host;
    fbdo->session.con_mode = fb_esp_con_mode_storage;
}

void FB_Storage::sendUploadCallback(FirebaseData *fbdo, FCS_UploadStatusInfo &in, FCS_UploadProgressCallback cb, FCS_UploadStatusInfo *out)
{

    fbdo->session.fcs.cbUploadInfo.status = in.status;
    fbdo->session.fcs.cbUploadInfo.errorMsg = in.errorMsg;
    fbdo->session.fcs.cbUploadInfo.progress = in.progress;
    fbdo->session.fcs.cbUploadInfo.localFileName = in.localFileName;
    fbdo->session.fcs.cbUploadInfo.remoteFileName = in.remoteFileName;
    fbdo->session.fcs.cbUploadInfo.fileSize = in.fileSize;
    fbdo->session.fcs.cbUploadInfo.elapsedTime = in.elapsedTime;

    if (cb)
        cb(fbdo->session.fcs.cbUploadInfo);

    if (out)
    {
        out->errorMsg = fbdo->session.fcs.cbUploadInfo.errorMsg;
        out->status = fbdo->session.fcs.cbUploadInfo.status;
        out->progress = fbdo->session.fcs.cbUploadInfo.progress;
        out->localFileName = fbdo->session.fcs.cbUploadInfo.localFileName;
    }
}

void FB_Storage::sendDownloadCallback(FirebaseData *fbdo, FCS_DownloadStatusInfo &in, FCS_DownloadProgressCallback cb, FCS_DownloadStatusInfo *out)
{

    fbdo->session.fcs.cbDownloadInfo.status = in.status;
    fbdo->session.fcs.cbDownloadInfo.errorMsg = in.errorMsg;
    fbdo->session.fcs.cbDownloadInfo.progress = in.progress;
    fbdo->session.fcs.cbDownloadInfo.localFileName = in.localFileName;
    fbdo->session.fcs.cbDownloadInfo.remoteFileName = in.remoteFileName;
    fbdo->session.fcs.cbDownloadInfo.fileSize = in.fileSize;
    fbdo->session.fcs.cbDownloadInfo.elapsedTime = in.elapsedTime;

    if (cb)
        cb(fbdo->session.fcs.cbDownloadInfo);

    if (out)
    {
        out->errorMsg = fbdo->session.fcs.cbDownloadInfo.errorMsg;
        out->status = fbdo->session.fcs.cbDownloadInfo.status;
        out->progress = fbdo->session.fcs.cbDownloadInfo.progress;
        out->localFileName = fbdo->session.fcs.cbDownloadInfo.localFileName;
    }
}

bool FB_Storage::fcs_connect(FirebaseData *fbdo)
{
    MB_String host = fb_esp_pgm_str_265;
    host += fb_esp_pgm_str_120;
    rescon(fbdo, host.c_str());
    fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
    return true;
}

void FB_Storage::reportUploadProgress(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req, size_t readBytes)
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

        fbdo->session.fcs.cbUploadInfo.status = fb_esp_fcs_upload_status_upload;
        FCS_UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_fcs_upload_status_upload;
        in.progress = p;
        in.elapsedTime = fbdo->tcpClient.dataTime;
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }
}

void FB_Storage::reportDownloadProgress(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req, size_t readBytes)
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

        fbdo->session.fcs.cbDownloadInfo.status = fb_esp_fcs_download_status_download;
        FCS_DownloadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_fcs_download_status_download;
        in.progress = p;
        in.fileSize = req->fileSize;
        in.elapsedTime = fbdo->tcpClient.dataTime;
        sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
    }
}

bool FB_Storage::fcs_sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req)
{

    fbdo->session.fcs.requestType = req->requestType;

    fbdo->session.http_code = 0;

    int ret = 0;

    if (req->requestType == fb_esp_fcs_request_type_upload)
        ret = ut->mbfs->open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);

    // Close file and open later.
    // This is inefficient unless less memory usage than keep file opened
    // which causes the issue in ESP32 core 2.0.x
    if (ret > 0)
        ut->mbfs->close(mbfs_type req->storageType); // fixed for ESP32 core v2.0.2, SPIFFS file
    else if (ret < 0)
    {
        fbdo->session.response.code = ret;
        return false;
    }

    req->fileSize = ret;

    MB_String header;

    size_t len = 0;

    ret = -1;

    if (req->requestType == fb_esp_fcs_request_type_upload || req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
        header += fb_esp_pgm_str_24;
    else if (req->requestType == fb_esp_fcs_request_type_download || req->requestType == fb_esp_fcs_request_type_download_ota || req->requestType == fb_esp_fcs_request_type_get_meta || req->requestType == fb_esp_fcs_request_type_list)
        header += fb_esp_pgm_str_25;
    else if (req->requestType == fb_esp_fcs_request_type_delete)
        header += fb_esp_pgm_str_27;

    header += fb_esp_pgm_str_6;
    header += fb_esp_pgm_str_266;
    header += req->bucketID;
    header += fb_esp_pgm_str_267;

    if (req->requestType == fb_esp_fcs_request_type_download || req->requestType == fb_esp_fcs_request_type_download_ota)
    {
        header += fb_esp_pgm_str_1;
        header += ut->url_encode(req->remoteFileName);
        header += fb_esp_pgm_str_173;
        header += fb_esp_pgm_str_269;
    }
    else if (req->requestType != fb_esp_fcs_request_type_list)
    {
        header += fb_esp_pgm_str_173;
        header += fb_esp_pgm_str_268;

        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);
    }

    header += fb_esp_pgm_str_30;

    if (req->requestType == fb_esp_fcs_request_type_upload || req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
    {
        header += fb_esp_pgm_str_8;
        header += req->mime;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_12;

        if (req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
            len = req->pgmArcLen;
        else if (req->requestType == fb_esp_fcs_request_type_upload)
            len = req->fileSize;

        header += len;
        header += fb_esp_pgm_str_21;
    }

    header += fb_esp_pgm_str_31;
    header += fb_esp_pgm_str_265;
    header += fb_esp_pgm_str_120;
    header += fb_esp_pgm_str_21;

    if (!Signer.getCfg()->signer.test_mode)
    {
        header += fb_esp_pgm_str_237;
        int type = Signer.getTokenType();
        if (type == token_type_id_token || type == token_type_custom_token)
            header += fb_esp_pgm_str_270;
        else if (type == token_type_oauth2_access_token)
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

    header += fb_esp_pgm_str_32;
    header += fb_esp_pgm_str_34;

    ut->getCustomHeaders(header);

    header += fb_esp_pgm_str_21;

    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    if (req->requestType == fb_esp_fcs_request_type_upload || req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
    {
        FCS_UploadStatusInfo in;
        in.localFileName = req->localFileName;
        in.remoteFileName = req->remoteFileName;
        in.status = fb_esp_fcs_upload_status_init;
        in.fileSize = req->fileSize;
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }

    fbdo->tcpClient.send(header.c_str());
    header.clear();

    if (fbdo->session.response.code > 0)
    {
        fbdo->session.fcs.storage_type = req->storageType;
        fbdo->session.connected = true;
        if (req->requestType == fb_esp_fcs_request_type_upload)
        {
            int available = len;

            // This is inefficient unless less memory usage than keep file opened
            // which causes the issue in ESP32 core 2.0.x
            ut->mbfs->open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);

            int bufLen = Signer.getCfg()->fcs.upload_buffer_size;
            if (bufLen < 512)
                bufLen = 512;

            if (bufLen > 1024 * 16)
                bufLen = 1024 * 16;

            uint8_t *buf = (uint8_t *)ut->newP(bufLen + 1, false);
            int read = 0;
            int readCount = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;

                read = ut->mbfs->read(mbfs_type req->storageType, buf, available);

                if (fbdo->tcpClient.write(buf, read) != read)
                    break;

                readCount += read;

                reportUploadProgress(fbdo, req, readCount);

                available = ut->mbfs->available(mbfs_type req->storageType);
            }

            ut->delP(&buf);

            ut->mbfs->close(mbfs_type req->storageType);

            reportUploadProgress(fbdo, req, req->fileSize);
        }
        else if (req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
        {
            int len = req->pgmArcLen;
            int available = len;
            req->fileSize = len;

            int bufLen = Signer.getCfg()->fcs.upload_buffer_size;
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

                reportUploadProgress(fbdo, req, pos);
                pos += available;
                len -= available;
                available = len;
            }

            ut->delP(&buf);
            reportUploadProgress(fbdo, req, pos);
        }

        bool res = handleResponse(fbdo, req);
        fbdo->closeSession();

        if (req->requestType == fb_esp_fcs_request_type_download)
            ut->mbfs->close(mbfs_type req->storageType);

        if (res)
        {
            if (req->requestType == fb_esp_fcs_request_type_download || req->requestType == fb_esp_fcs_request_type_download_ota)
            {
                FCS_DownloadStatusInfo in;
                in.localFileName = req->localFileName;
                in.remoteFileName = req->remoteFileName;
                in.status = fb_esp_fcs_download_status_complete;
                in.fileSize = req->fileSize;
                in.progress = 100;
                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
            }
            else if (req->requestType == fb_esp_fcs_request_type_upload || req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
            {
                FCS_UploadStatusInfo in;
                in.localFileName = req->localFileName;
                in.remoteFileName = req->remoteFileName;
                in.status = fb_esp_fcs_upload_status_complete;
                in.fileSize = req->fileSize;
                in.progress = 100;
                sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
            }
        }

        Signer.getCfg()->internal.fb_processing = false;
        return res;
    }
    else
        fbdo->session.connected = false;

    if (req->requestType == fb_esp_fcs_request_type_download)
        ut->mbfs->close(mbfs_type req->storageType);

    Signer.getCfg()->internal.fb_processing = false;

    return true;
}

bool FB_Storage::handleResponse(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req)
{

#ifdef ENABLE_RTDB
    if (fbdo->session.rtdb.pause)
        return true;
#endif
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
    MB_String tmp1, tmp2, tmp3, part1, part2;
    size_t p1 = 0;
    size_t p2 = 0;

    if (req->requestType == fb_esp_fcs_request_type_list)
    {
        tmp1 = fb_esp_pgm_str_476;
        tmp2 = fb_esp_pgm_str_477;
        tmp3 = fb_esp_pgm_str_3;
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

    int availablePayload = chunkBufSize;

    dataTime = millis();
#ifdef ENABLE_FB_FUNCTIONS
    fbdo->session.cfn.payload.clear();
#endif

    if (req->requestType == fb_esp_fcs_request_type_download)
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
            fbdo->tcpClient.flush();
            fbdo->session.response.code = ret;
            return false;
        }
#endif
    }

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
                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.contentLen > 0 && (fbdo->session.fcs.requestType == fb_esp_fcs_request_type_download || fbdo->session.fcs.requestType == fb_esp_fcs_request_type_download_ota))
                            {

                                size_t available = fbdo->tcpClient.available();
                                dataTime = millis();

                                req->fileSize = response.contentLen;
                                error.code = 0;

                                FCS_DownloadStatusInfo in;
                                in.localFileName = req->localFileName;
                                in.remoteFileName = req->remoteFileName;
                                in.status = fb_esp_fcs_download_status_init;
                                in.fileSize = req->fileSize;
                                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);

                                if (fbdo->session.fcs.requestType == fb_esp_fcs_request_type_download_ota)
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

                                int bufLen = Signer.getCfg()->fcs.download_buffer_size;
                                if (bufLen < 512)
                                    bufLen = 512;

                                if (bufLen > 1024 * 16)
                                    bufLen = 1024 * 16;

                                uint8_t *buf = (uint8_t *)ut->newP(bufLen + 1, false);

                                while (fbdo->reconnect(dataTime) && fbdo->tcpClient.connected() && payloadRead < response.contentLen)
                                {

                                    if (available)
                                    {
                                        fbdo->session.payload_length += available;
                                        if (fbdo->session.max_payload_length < fbdo->session.payload_length)
                                            fbdo->session.max_payload_length = fbdo->session.payload_length;
                                        dataTime = millis();

                                        if ((int)available > bufLen)
                                            available = bufLen;

                                        size_t read = fbdo->tcpClient.readBytes(buf, available);
                                        if (read == available)
                                        {
                                            reportDownloadProgress(fbdo, req, payloadRead);

                                            if (fbdo->session.fcs.requestType == fb_esp_fcs_request_type_download_ota)
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

                                ut->delP(&buf);

                                reportDownloadProgress(fbdo, req, payloadRead);

                                if (fbdo->session.fcs.requestType == fb_esp_fcs_request_type_download_ota)
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

                                    if (fbdo->session.fcs.requestType == fb_esp_fcs_request_type_list)
                                    {

                                        delay(10);
                                        part1 = pChunk;
                                        p1 = part1.find(tmp1);
                                        if (p1 != MB_String::npos)
                                        {
                                            p2 = part1.find(tmp3, p1 + tmp1.length());
                                            if (p2 != MB_String::npos)
                                                part2 = part1.substr(p1 + tmp1.length(), p2 - p1 - tmp1.length());
                                        }
                                        else
                                        {
                                            p1 = part1.find(tmp2);
                                            if (p1 != MB_String::npos)
                                            {
                                                p2 = part1.find(tmp3, p1 + tmp2.length());
                                                if (p2 != MB_String::npos)
                                                {
                                                    fb_esp_fcs_file_list_item_t itm;
                                                    itm.name = part2;
                                                    itm.bucket = part1.substr(p1 + tmp2.length(), p2 - p1 - tmp2.length());
                                                    fbdo->session.fcs.files.items.push_back(itm);
                                                    part2.clear();
                                                    part1.clear();
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
                    if (fbdo->session.fcs.requestType != fb_esp_fcs_request_type_list)
                    {
                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_274))
                            fbdo->session.fcs.meta.name = fbdo->session.dataPtr->to<const char *>();

                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_275))
                            fbdo->session.fcs.meta.bucket = fbdo->session.dataPtr->to<const char *>();

                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_276))
                            fbdo->session.fcs.meta.generation = atof(fbdo->session.dataPtr->to<const char *>());

                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_277))
                            fbdo->session.fcs.meta.contentType = fbdo->session.dataPtr->to<const char *>();

                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_278))
                            fbdo->session.fcs.meta.size = atoi(fbdo->session.dataPtr->to<const char *>());

                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_279))
                            fbdo->session.fcs.meta.etag = fbdo->session.dataPtr->to<const char *>();

                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_280))
                            fbdo->session.fcs.meta.crc32 = fbdo->session.dataPtr->to<const char *>();

                        if (parseJsonResponse(fbdo, fb_esp_pgm_str_272))
                            fbdo->session.fcs.meta.downloadTokens = fbdo->session.dataPtr->to<const char *>();
                    }
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

bool FB_Storage::parseJsonResponse(FirebaseData *fbdo, PGM_P key_path)
{
    fbdo->session.dataPtr->clear();
    fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(key_path));
    return fbdo->session.dataPtr->success;
}

#endif

#endif // ENABLE