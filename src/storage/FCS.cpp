/**
 * Google's Firebase Storage class, FCS.cpp version 1.0.6
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created March 13, 2021
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

    fcs_connect(fbdo);
    fbdo->_ss.fcs.meta.name.clear();
    fbdo->_ss.fcs.meta.bucket.clear();
    fbdo->_ss.fcs.meta.contentType.clear();
    fbdo->_ss.fcs.meta.crc32.clear();
    fbdo->_ss.fcs.meta.etag.clear();
    fbdo->_ss.fcs.meta.downloadTokens.clear();
    fbdo->_ss.fcs.meta.generation = 0;
    fbdo->_ss.fcs.meta.size = 0;

    if (req->requestType == fb_esp_fcs_request_type_download || req->requestType == fb_esp_fcs_request_type_upload)
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

    return fcs_sendRequest(fbdo, req);
}

bool FB_Storage::upload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, const char *remoteFileName, const char *mime)
{
    struct fb_esp_fcs_req_t req;
    req.localFileName = localFileName;
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.requestType = fb_esp_fcs_request_type_upload;
    req.bucketID = bucketID;
    req.mime = mime;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::upload(FirebaseData *fbdo, const char *bucketID, const uint8_t *data, size_t len, const char *remoteFileName, const char *mime)
{
    struct fb_esp_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.requestType = fb_esp_fcs_request_type_upload_pgm_data;
    req.bucketID = bucketID;
    req.mime = mime;
    req.pgmArc = data;
    req.pgmArcLen = len;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::download(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType)
{
    struct fb_esp_fcs_req_t req;
    req.localFileName = localFileName;
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.requestType = fb_esp_fcs_request_type_download;
    req.bucketID = bucketID;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::getMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName)
{
    struct fb_esp_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;
    req.requestType = fb_esp_fcs_request_type_get_meta;
    return sendRequest(fbdo, &req);
}

bool FB_Storage::deleteFile(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName)
{
    struct fb_esp_fcs_req_t req;
    req.requestType = fb_esp_fcs_request_type_delete;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;

    return sendRequest(fbdo, &req);
}

bool FB_Storage::listFiles(FirebaseData *fbdo, const char *bucketID)
{
    struct fb_esp_fcs_req_t req;
    req.bucketID = bucketID;
    req.requestType = fb_esp_fcs_request_type_list;

    return sendRequest(fbdo, &req);
}

void FB_Storage::rescon(FirebaseData *fbdo, const char *host)
{
    if (!fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.con_mode != fb_esp_con_mode_storage || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->_ss.host = host;
    fbdo->_ss.con_mode = fb_esp_con_mode_storage;
}

bool FB_Storage::fcs_connect(FirebaseData *fbdo)
{
    std::string host;
    ut->appendP(host, fb_esp_pgm_str_265);
    ut->appendP(host, fb_esp_pgm_str_120);
    rescon(fbdo, host.c_str());
    fbdo->httpClient.begin(host.c_str(), 443);
    return true;
}

bool FB_Storage::fcs_sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req)
{

    fbdo->_ss.fcs.requestType = req->requestType;

    std::string token = Signer.getToken(Signer.getTokenType());

    if (!Signer.getCfg()->_int.fb_flash_rdy)
        ut->flashTest();

    if (req->requestType == fb_esp_fcs_request_type_download)
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
        if (req->requestType == fb_esp_fcs_request_type_upload)
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
        }
    }

    std::string header;

    if (req->requestType == fb_esp_fcs_request_type_upload || req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
        ut->appendP(header, fb_esp_pgm_str_24);
    else if (req->requestType == fb_esp_fcs_request_type_download || req->requestType == fb_esp_fcs_request_type_get_meta || req->requestType == fb_esp_fcs_request_type_list)
        ut->appendP(header, fb_esp_pgm_str_25);
    else if (req->requestType == fb_esp_fcs_request_type_delete)
        ut->appendP(header, fb_esp_pgm_str_27);

    ut->appendP(header, fb_esp_pgm_str_6);
    ut->appendP(header, fb_esp_pgm_str_266);
    header += req->bucketID;
    ut->appendP(header, fb_esp_pgm_str_267);

    if (req->requestType == fb_esp_fcs_request_type_download)
    {
        if (req->remoteFileName[0] != '/')
            ut->appendP(header, fb_esp_pgm_str_1);

        header += ut->url_encode(req->remoteFileName);
        ut->appendP(header, fb_esp_pgm_str_173);
        ut->appendP(header, fb_esp_pgm_str_269);
    }
    else if (req->requestType != fb_esp_fcs_request_type_list)
    {
        ut->appendP(header, fb_esp_pgm_str_173);
        ut->appendP(header, fb_esp_pgm_str_268);

        if (req->remoteFileName[0] == '/')
            header += ut->url_encode(req->remoteFileName.substr(1, req->remoteFileName.length() - 1));
        else
            header += ut->url_encode(req->remoteFileName);
    }

    ut->appendP(header, fb_esp_pgm_str_30);

    if (req->requestType == fb_esp_fcs_request_type_upload || req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
    {
        ut->appendP(header, fb_esp_pgm_str_8);
        header += req->mime;
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_12);

        size_t len = 0;
        if (req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
            len = req->pgmArcLen;
        else if (req->requestType == fb_esp_fcs_request_type_upload)
            len = req->fileSize;

        char *tmp = ut->intStr(len);

        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);
    }

    ut->appendP(header, fb_esp_pgm_str_31);
    ut->appendP(header, fb_esp_pgm_str_265);
    ut->appendP(header, fb_esp_pgm_str_120);
    ut->appendP(header, fb_esp_pgm_str_21);

    ut->appendP(header, fb_esp_pgm_str_237);
    int type = Signer.getTokenType();
    if (type == token_type_id_token || type == token_type_custom_token)
        ut->appendP(header, fb_esp_pgm_str_270);
    else if (type == token_type_oauth2_access_token)
        ut->appendP(header, fb_esp_pgm_str_271);

    header += token;
    ut->appendP(header, fb_esp_pgm_str_21);
    ut->appendP(header, fb_esp_pgm_str_32);
    ut->appendP(header, fb_esp_pgm_str_34);
    ut->appendP(header, fb_esp_pgm_str_21);

    fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;

    int ret = fbdo->httpClient.send(header.c_str(), "");

    if (ret == 0)
    {
        fbdo->_ss.connected = true;
        if (Signer.getCfg()->_int.fb_file && req->requestType == fb_esp_fcs_request_type_upload)
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
                if (fbdo->httpClient.stream()->write(buf, read) != read)
                    break;
                available = Signer.getCfg()->_int.fb_file.available();
            }
            delete[] buf;
            Signer.getCfg()->_int.fb_file.close();
        }
        else if (req->requestType == fb_esp_fcs_request_type_upload_pgm_data)
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
                if (fbdo->httpClient.stream()->write(buf, available) != (size_t)available)
                    break;
                pos += available;
                len -= available;
                available = len;
            }
            delete[] buf;
        }

        ret = handleResponse(fbdo);
        fbdo->closeSession();

        if (Signer.getCfg()->_int.fb_file && req->requestType == fb_esp_fcs_request_type_download)
            Signer.getCfg()->_int.fb_file.close();

        Signer.getCfg()->_int.fb_processing = false;
        return ret;
    }
    else
        fbdo->_ss.connected = false;

    if (Signer.getCfg()->_int.fb_file && req->requestType == fb_esp_fcs_request_type_download)
        Signer.getCfg()->_int.fb_file.close();

    Signer.getCfg()->_int.fb_processing = false;

    return false;
}

bool FB_Storage::handleResponse(FirebaseData *fbdo)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

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
    std::string part1, part2;
    size_t p1 = 0;
    size_t p2 = 0;
    char *tmp1 = ut->strP(fb_esp_pgm_str_476);
    char *tmp2 = ut->strP(fb_esp_pgm_str_477);
    char *tmp3 = ut->strP(fb_esp_pgm_str_3);
    std::string payload;
    fbdo->_ss.fcs.files.items.clear();

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

                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
                                error.code = 0;

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
                            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && response.contentLen > 0 && fbdo->_ss.fcs.requestType == fb_esp_fcs_request_type_download)
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

                                    if (fbdo->_ss.fcs.requestType == fb_esp_fcs_request_type_list)
                                    {
                                        delay(10);
                                        part1 = pChunk;
                                        p1 = part1.find(tmp1);
                                        if (p1 != std::string::npos)
                                        {
                                            p2 = part1.find(tmp3, p1 + strlen(tmp1));
                                            if (p2 != std::string::npos)
                                                part2 = part1.substr(p1 + strlen(tmp1), p2 - p1 - strlen(tmp1));
                                        }
                                        else
                                        {
                                            p1 = part1.find(tmp2);
                                            if (p1 != std::string::npos)
                                            {
                                                p2 = part1.find(tmp3, p1 + strlen(tmp2));
                                                if (p2 != std::string::npos)
                                                {
                                                    fb_esp_fcs_file_list_item_t itm;
                                                    itm.name = part2;
                                                    itm.bucket = part1.substr(p1 + strlen(tmp2), p2 - p1 - strlen(tmp2));
                                                    fbdo->_ss.fcs.files.items.push_back(itm);
                                                    part2.clear();
                                                    part1.clear();
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
                        }
                    }
                }

                chunkIdx++;
            }
        }

        ut->delS(tmp1);
        ut->delS(tmp2);
        ut->delS(tmp3);

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
                    if (fbdo->_ss.fcs.requestType != fb_esp_fcs_request_type_list)
                    {
                        tmp = ut->strP(fb_esp_pgm_str_274);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.name = fbdo->_ss.data.stringValue.c_str();

                        tmp = ut->strP(fb_esp_pgm_str_275);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.bucket = fbdo->_ss.data.stringValue.c_str();

                        tmp = ut->strP(fb_esp_pgm_str_276);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.generation = atof(fbdo->_ss.data.stringValue.c_str());

                        tmp = ut->strP(fb_esp_pgm_str_277);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.contentType = fbdo->_ss.data.stringValue.c_str();

                        tmp = ut->strP(fb_esp_pgm_str_278);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.size = atoi(fbdo->_ss.data.stringValue.c_str());

                        tmp = ut->strP(fb_esp_pgm_str_279);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.etag = fbdo->_ss.data.stringValue.c_str();

                        tmp = ut->strP(fb_esp_pgm_str_280);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.crc32 = fbdo->_ss.data.stringValue.c_str();

                        tmp = ut->strP(fb_esp_pgm_str_272);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.fcs.meta.downloadTokens = fbdo->_ss.data.stringValue.c_str();
                    }
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