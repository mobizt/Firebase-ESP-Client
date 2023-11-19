
/**
 * Google's Firebase Storage class, FCS.cpp version 1.2.13
 *
 * This library supports Espressif ESP8266, ESP32 and RP2040 Pico
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

#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)

#ifndef FB_Storage_CPP
#define FB_Storage_CPP

#include "FCS.h"

FB_Storage::FB_Storage()
{
}
FB_Storage ::~FB_Storage()
{
}

bool FB_Storage::sendRequest(FirebaseData *fbdo, struct firebase_fcs_req_t *req)
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

    fcs_connect(fbdo);
    fbdo->session.fcs.meta.name.clear();
    fbdo->session.fcs.meta.bucket.clear();
    fbdo->session.fcs.meta.contentType.clear();
    fbdo->session.fcs.meta.crc32.clear();
    fbdo->session.fcs.meta.etag.clear();
    fbdo->session.fcs.meta.downloadTokens.clear();
    fbdo->session.fcs.meta.generation = 0;
    fbdo->session.fcs.meta.size = 0;

    if (req->requestType == firebase_fcs_request_type_download ||
        req->requestType == firebase_fcs_request_type_download_ota ||
        req->requestType == firebase_fcs_request_type_upload)
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

    Core.internal.fb_processing = true;

    bool ret = fcs_sendRequest(fbdo, req);

    Core.internal.fb_processing = false;

    if (!ret)
    {
        if (req->requestType == firebase_fcs_request_type_download ||
            req->requestType == firebase_fcs_request_type_download_ota)
        {
            fbdo->session.fcs.cbDownloadInfo.status = firebase_fcs_download_status_error;
            FCS_DownloadStatusInfo in;
            makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_download_status_error,
                               0, 0, 0, fbdo->errorReason());
            sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
        }
        else if (req->requestType == firebase_fcs_request_type_upload ||
                 req->requestType == firebase_fcs_request_type_upload_pgm_data)
        {
            fbdo->session.fcs.cbUploadInfo.status = firebase_fcs_upload_status_error;
            FCS_UploadStatusInfo in;
            makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_upload_status_error,
                             0, 0, 0, fbdo->errorReason());
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        }
        fbdo->closeSession();
    }

    return ret;
}

bool FB_Storage::mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr localFileName,
                         firebase_mem_storage_type storageType, MB_StringPtr remoteFileName,
                         MB_StringPtr mime, FCS_UploadProgressCallback callback)
{
    struct firebase_fcs_req_t req;
    req.localFileName = localFileName;
    Core.ut.makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.requestType = firebase_fcs_request_type_upload;
    req.bucketID = bucketID;
    req.mime = mime;
    req.uploadCallback = callback;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, const uint8_t *data, size_t len,
                         MB_StringPtr remoteFileName, MB_StringPtr mime,
                         FCS_UploadProgressCallback callback)
{
    struct firebase_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.requestType = firebase_fcs_request_type_upload_pgm_data;
    req.bucketID = bucketID;
    req.mime = mime;
    req.pgmArc = data;
    req.pgmArcLen = len;
    req.uploadCallback = callback;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mDownload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName,
                           MB_StringPtr localFileName, firebase_mem_storage_type storageType,
                           FCS_DownloadProgressCallback callback)
{
    struct firebase_fcs_req_t req;
    req.localFileName = localFileName;
    Core.ut.makePath(req.localFileName);
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.requestType = firebase_fcs_request_type_download;
    req.bucketID = bucketID;
    req.downloadCallback = callback;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mDownloadOTA(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName,
                              FCS_DownloadProgressCallback callback)
{
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))
    struct firebase_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.requestType = firebase_fcs_request_type_download_ota;
    req.bucketID = bucketID;
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

bool FB_Storage::mGetMetadata(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName)
{
    struct firebase_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;
    req.requestType = firebase_fcs_request_type_get_meta;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mDeleteFile(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName)
{
    struct firebase_fcs_req_t req;
    req.requestType = firebase_fcs_request_type_delete;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::mListFiles(FirebaseData *fbdo, MB_StringPtr bucketID)
{
    struct firebase_fcs_req_t req;
    req.bucketID = bucketID;
    req.requestType = firebase_fcs_request_type_list;
    return sendRequest(fbdo, &req);
}

void FB_Storage::rescon(FirebaseData *fbdo, const char *host)
{
    fbdo->_responseCallback = NULL;

    if (fbdo->session.cert_updated || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout ||
        fbdo->session.con_mode != firebase_con_mode_storage || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->session.host = host;
    fbdo->session.con_mode = firebase_con_mode_storage;
}

void FB_Storage::sendUploadCallback(FirebaseData *fbdo, FCS_UploadStatusInfo &in,
                                    FCS_UploadProgressCallback cb, FCS_UploadStatusInfo *out)
{
    fbdo->session.fcs.cbUploadInfo = in;
    if (cb)
        cb(fbdo->session.fcs.cbUploadInfo);
    if (out)
        out = &fbdo->session.fcs.cbUploadInfo;
}

void FB_Storage::sendDownloadCallback(FirebaseData *fbdo, FCS_DownloadStatusInfo &in,
                                      FCS_DownloadProgressCallback cb, FCS_DownloadStatusInfo *out)
{
    fbdo->session.fcs.cbDownloadInfo = in;
    if (cb)
        cb(fbdo->session.fcs.cbDownloadInfo);
    if (out)
        out = &fbdo->session.fcs.cbDownloadInfo;
}

void FB_Storage::makeUploadStatus(FCS_UploadStatusInfo &info, const MB_String &local,
                                  const MB_String &remote, firebase_fcs_upload_status status, size_t progress,
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

void FB_Storage::makeDownloadStatus(FCS_DownloadStatusInfo &info, const MB_String &local,
                                    const MB_String &remote, firebase_fcs_download_status status, size_t progress,
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

bool FB_Storage::fcs_connect(FirebaseData *fbdo)
{
    MB_String host;
    Core.hh.addGAPIsHost(host, firebase_storage_ss_pgm_str_1 /* "firebasestorage." */);
    rescon(fbdo, host.c_str());
    fbdo->tcpClient.setSession(&fbdo->bsslSession);
    fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
    return true;
}

void FB_Storage::reportUploadProgress(FirebaseData *fbdo, struct firebase_fcs_req_t *req, size_t readBytes)
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
        fbdo->session.fcs.cbUploadInfo.status = firebase_fcs_upload_status_upload;
        FCS_UploadStatusInfo in;
        makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_upload_status_upload,
                         p, 0, fbdo->tcpClient.dataTime, "");
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }
}

void FB_Storage::reportDownloadProgress(FirebaseData *fbdo, struct firebase_fcs_req_t *req, size_t readBytes)
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
        fbdo->session.fcs.cbDownloadInfo.status = firebase_fcs_download_status_download;
        FCS_DownloadStatusInfo in;
        makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_download_status_download,
                           p, req->fileSize, fbdo->tcpClient.dataTime, "");
        sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
    }
}

bool FB_Storage::fcs_sendRequest(FirebaseData *fbdo, struct firebase_fcs_req_t *req)
{

    fbdo->session.fcs.requestType = req->requestType;
    fbdo->session.http_code = 0;
    int ret = 0;

    if (req->requestType == firebase_fcs_request_type_upload)
        ret = Core.mbfs.open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);
    if (ret > 0)
        Core.mbfs.close(mbfs_type req->storageType); // fixed for ESP32 core v2.0.2, SPIFFS file
    else if (ret < 0)
    {
        fbdo->session.response.code = ret;
        return false;
    }

    req->fileSize = ret;

    MB_String header;
    size_t len = 0;
    bool hasParams = false;
    ret = -1;
    firebase_request_method method = http_undefined;

    if (req->requestType == firebase_fcs_request_type_upload || req->requestType == firebase_fcs_request_type_upload_pgm_data)
        method = http_post;
    else if (req->requestType == firebase_fcs_request_type_download ||
             req->requestType == firebase_fcs_request_type_download_ota ||
             req->requestType == firebase_fcs_request_type_get_meta ||
             req->requestType == firebase_fcs_request_type_list)
        method = http_get;
    else if (req->requestType == firebase_fcs_request_type_delete)
        method = http_delete;

    if (method != http_undefined)
        Core.hh.addRequestHeaderFirst(header, method);

    header += firebase_storage_ss_pgm_str_2; // "/v0/b/"
    header += req->bucketID;
    header += firebase_storage_ss_pgm_str_3; // "/o"

    if (req->requestType == firebase_fcs_request_type_download || req->requestType == firebase_fcs_request_type_download_ota)
    {
        header += firebase_pgm_str_1; // "/"
        header += Core.uh.encode(req->remoteFileName);
        header += firebase_pgm_str_7;            // "?"
        header += firebase_storage_ss_pgm_str_4; // "alt=media"
    }
    else if (req->requestType != firebase_fcs_request_type_list)
        Core.uh.addParam(header, firebase_storage_pgm_str_1 /* "name=" */,
                            req->remoteFileName[0] == '/'
                                ? Core.uh.encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1))
                                : Core.uh.encode(req->remoteFileName),
                            hasParams);

    Core.hh.addRequestHeaderLast(header);

    if (req->requestType == firebase_fcs_request_type_upload || req->requestType == firebase_fcs_request_type_upload_pgm_data)
    {
        Core.hh.addContentTypeHeader(header, req->mime.c_str());

        if (req->requestType == firebase_fcs_request_type_upload_pgm_data)
            len = req->pgmArcLen;
        else if (req->requestType == firebase_fcs_request_type_upload)
            len = req->fileSize;

        Core.hh.addContentLengthHeader(header, len);
    }

    Core.hh.addGAPIsHostHeader(header, firebase_storage_ss_pgm_str_1 /* "firebasestorage." */);

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
    Core.hh.addUAHeader(header);
    // required for ESP32 core sdk v2.0.x.
    bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE) || defined(FIREBASE_USE_CONNECTION_KEEP_ALIVE_MODE)
    keepAlive = true;
#endif
    Core.hh.addConnectionHeader(header, keepAlive);

    Core.hh.getCustomHeaders(&Core.sh,header, Core.config->signer.customHeaders);
    Core.hh.addNewLine(header);

    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    if (req->requestType == firebase_fcs_request_type_upload || req->requestType == firebase_fcs_request_type_upload_pgm_data)
    {
        FCS_UploadStatusInfo in;
        makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_upload_status_init, 0, req->fileSize, 0, "");
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }

    fbdo->tcpSend(header.c_str());
    header.clear();

    if (fbdo->session.response.code > 0)
    {
        fbdo->session.fcs.storage_type = req->storageType;
        bool waitResponse = true;
        if (req->requestType == firebase_fcs_request_type_upload)
        {
            int available = len;
            // Fix in ESP32 core 2.0.x
            Core.mbfs.open(req->localFileName, mbfs_type req->storageType, mb_fs_open_mode_read);

            int bufLen = Core.ut.getUploadBufSize(Core.config, firebase_con_mode_storage);
            uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen + 1, false));
            int read = 0;
            int readCount = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;

                read = Core.mbfs.read(mbfs_type req->storageType, buf, available);
                if (read && (int)fbdo->tcpWrite(buf, read) != read)
                    break;

                readCount += read;
                reportUploadProgress(fbdo, req, readCount);
                available = Core.mbfs.available(mbfs_type req->storageType);
            }

            Core.mbfs.delP(&buf);
            Core.mbfs.close(mbfs_type req->storageType);
            if (readCount == (int)req->fileSize)
                reportUploadProgress(fbdo, req, req->fileSize);
            else
                waitResponse = false;
        }
        else if (req->requestType == firebase_fcs_request_type_upload_pgm_data)
        {
            int len = req->pgmArcLen;
            int available = len;
            req->fileSize = len;

            int bufLen = Core.ut.getUploadBufSize(Core.config, firebase_con_mode_storage);
            uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen + 1, false));
            size_t pos = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                memcpy_P(buf, req->pgmArc + pos, available);
                if ((int)fbdo->tcpWrite(buf, available) != available)
                    break;

                reportUploadProgress(fbdo, req, pos);
                pos += available;
                len -= available;
                available = len;
            }
            Core.mbfs.delP(&buf);
            if (req->pgmArcLen == pos)
                reportUploadProgress(fbdo, req, pos);
            else
                waitResponse = false;
        }

        bool res = false;

        if (waitResponse)
        {
            res = handleResponse(fbdo, req);
            fbdo->closeSession();

            if (req->requestType == firebase_fcs_request_type_download)
                Core.mbfs.close(mbfs_type req->storageType);

            if (res)
            {
                if (req->requestType == firebase_fcs_request_type_download || req->requestType == firebase_fcs_request_type_download_ota)
                {
                    FCS_DownloadStatusInfo in;
                    makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_download_status_complete,
                                       100, req->fileSize, 0, "");
                    sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
                }
                else if (req->requestType == firebase_fcs_request_type_upload ||
                         req->requestType == firebase_fcs_request_type_upload_pgm_data)
                {
                    FCS_UploadStatusInfo in;
                    makeUploadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_upload_status_complete,
                                     100, req->fileSize, 0, "");
                    sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
                }
            }
        }
        else
            fbdo->closeSession();

        Core.internal.fb_processing = false;

        return res;
    }

    if (req->requestType == firebase_fcs_request_type_download)
        Core.mbfs.close(mbfs_type req->storageType);

    Core.internal.fb_processing = false;

    return false;
}

bool FB_Storage::handleResponse(FirebaseData *fbdo, struct firebase_fcs_req_t *req)
{

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
    if (fbdo->session.rtdb.pause)
        return true;
#endif
    if (!fbdo->reconnect())
        return false;

    bool isOTA = fbdo->session.fcs.requestType == firebase_fcs_request_type_download_ota;
    MB_String payload;
    struct server_response_data_t response;
    struct firebase_tcp_response_handler_t tcpHandler;

    Core.hh.initTCPSession(fbdo->session);
    Core.hh.intTCPHandler(&(fbdo->tcpClient), tcpHandler, 2048, fbdo->session.resp_size, &payload, isOTA);

    firebase_fcs_file_list_item_t itm;
    int fileInfoStage = 0;
    fbdo->session.fcs.files.items.clear();

    bool isList = false, isMeta = false;
    int upos = 0;

    if (!fbdo->waitResponse(tcpHandler))
        return false;

    if (!fbdo->tcpClient.connected())
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

#ifdef ENABLE_FB_FUNCTIONS
    fbdo->session.cfn.payload.clear();
#endif

    if (req->requestType == firebase_fcs_request_type_download && !fbdo->prepareDownload(req->localFileName, req->storageType, true))
        return false;

    bool complete = false;

    while (tcpHandler.available() > 0 /* data available to read payload */ ||
           tcpHandler.payloadRead < response.contentLen /* incomplete content read  */)
    {

        if (!fbdo->readResponse(nullptr, tcpHandler, response))
            break;

        if (tcpHandler.pChunkIdx > 0)
        {

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.contentLen > 0 &&
                (fbdo->session.fcs.requestType == firebase_fcs_request_type_download ||
                 fbdo->session.fcs.requestType == firebase_fcs_request_type_download_ota))
            {

                tcpHandler.dataTime = millis();

                req->fileSize = response.contentLen;
                tcpHandler.error.code = 0;

                FCS_DownloadStatusInfo in;
                makeDownloadStatus(in, req->localFileName, req->remoteFileName, firebase_fcs_download_status_init,
                                   0, req->fileSize, 0, "");
                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);

                int bufLen = Core.config->fcs.download_buffer_size;
                if (bufLen < 512)
                    bufLen = 512;

                if (bufLen > 1024 * 16)
                    bufLen = 1024 * 16;
                uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen, false));

                int stage = 0;

                if (isOTA)
                    fbdo->prepareDownloadOTA(tcpHandler, response);

                while (fbdo->processDownload(req->localFileName, req->storageType, buf, bufLen, tcpHandler, response, stage, isOTA))
                {
                    if (stage)
                        reportDownloadProgress(fbdo, req, tcpHandler.payloadRead);
                }

                Core.mbfs.delP(&buf);

                if (!isOTA)
                {
                    if ((int)req->fileSize == tcpHandler.payloadRead)
                        reportDownloadProgress(fbdo, req, tcpHandler.payloadRead);
                    else
                        tcpHandler.error.code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
                }

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
                    break;

                if (tcpHandler.bufferAvailable > 0 && pChunk.length() > 0)
                {
                    FBUtils::idle();

                    isList = fbdo->session.fcs.requestType == firebase_fcs_request_type_list;
                    isMeta = fbdo->session.fcs.requestType != firebase_fcs_request_type_download_ota &&
                             fbdo->session.fcs.requestType != firebase_fcs_request_type_download;

                    if (fbdo->getUploadInfo(0, fileInfoStage, pChunk, isList, isMeta, &itm, upos))
                        fbdo->session.fcs.files.items.push_back(itm);

                    payload += pChunk;
                }
            }
        }
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    fbdo->getAllUploadInfo(0, fileInfoStage, payload, isList, isMeta, &itm);

    // parse payload for error
    fbdo->getError(payload, tcpHandler, response, true);

    return tcpHandler.error.code == 0;
}

bool FB_Storage::parseJsonResponse(FirebaseData *fbdo, PGM_P key_path)
{
    return Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, key_path);
}

#endif

#endif // ENABLE