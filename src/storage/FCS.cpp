/**
 * Google's Firebase Cloud Storage class, FCS.cpp version 1.0.1
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 29, 2021
 * 
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2020, 2021 K. Suwatchai (Mobizt)
 * 
 * The MIT License (MIT)
 * Copyright (c) 2020, 2021 K. Suwatchai (Mobizt)
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

#ifndef FB_CloudStorage_CPP
#define FB_CloudStorage_CPP

#include "FCS.h"

FB_CloudStorage::FB_CloudStorage()
{
}
FB_CloudStorage ::~FB_CloudStorage()
{
}

void FB_CloudStorage::begin(UtilsClass *u)
{
    ut = u;
}

bool FB_CloudStorage::sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req)
{
    if (!Signer.tokenReady())
        return false;

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

bool FB_CloudStorage::upload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, const char *remoteFileName, const char *mime)
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

bool FB_CloudStorage::download(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType)
{
    struct fb_esp_fcs_req_t req;
    req.localFileName = localFileName;
    req.remoteFileName = remoteFileName;
    req.storageType = storageType;
    req.requestType = fb_esp_fcs_request_type_download;
    req.bucketID = bucketID;
    return sendRequest(fbdo, &req);
}

bool FB_CloudStorage::getMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName)
{
    struct fb_esp_fcs_req_t req;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;
    req.requestType = fb_esp_fcs_request_type_get_meta;
    return sendRequest(fbdo, &req);
}

bool FB_CloudStorage::deleteFile(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName)
{
    struct fb_esp_fcs_req_t req;
    req.requestType = fb_esp_fcs_request_type_delete;
    req.remoteFileName = remoteFileName;
    req.bucketID = bucketID;

    return sendRequest(fbdo, &req);
}

bool FB_CloudStorage::listFiles(FirebaseData *fbdo, const char *bucketID)
{
    struct fb_esp_fcs_req_t req;
    req.bucketID = bucketID;
    req.requestType = fb_esp_fcs_request_type_list;

    return sendRequest(fbdo, &req);
}

bool FB_CloudStorage::fcs_connect(FirebaseData *fbdo)
{

    if (!fbdo->_ss.connected || fbdo->_ss.con_mode != fb_esp_con_mode_storage)
    {
        fbdo->closeSession();
        fbdo->setSecure();
    }

    std::string host;
    ut->appendP(host, fb_esp_pgm_str_265);
    ut->appendP(host, fb_esp_pgm_str_120);
    fbdo->httpClient.begin(host.c_str(), 443);
    fbdo->_ss.con_mode = fb_esp_con_mode_storage;
    return true;
}

bool FB_CloudStorage::fcs_sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req)
{

    fbdo->_ss.fcs.requestType = req->requestType;

    std::string token = Signer.getToken(Signer.getTokenType());

    if (!Signer.getCfg()->_int.fb_flash_rdy)
        Signer.getCfg()->_int.fb_flash_rdy = FLASH_FS.begin();

    if (req->requestType == fb_esp_fcs_request_type_download)
    {
        if (req->storageType == mem_storage_type_sd)
        {
            if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            {
                fbdo->_ss.http_code = FIREBASE_ERROR_FILE_IO_ERROR;
                return false;
            }
            Signer.getCfg()->_int.fb_file = SD.open(req->localFileName.c_str(), FILE_WRITE);
        }
        else if (req->storageType == mem_storage_type_flash)
        {
            if (!Signer.getCfg()->_int.fb_flash_rdy)
                Signer.getCfg()->_int.fb_flash_rdy = FLASH_FS.begin();
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
                   
                Signer.getCfg()->_int.fb_file = SD.open(req->localFileName.c_str(), FILE_READ);
            }
            else if (req->storageType == mem_storage_type_flash)
            {
                if (!Signer.getCfg()->_int.fb_flash_rdy)
                    Signer.getCfg()->_int.fb_flash_rdy = FLASH_FS.begin();
                Signer.getCfg()->_int.fb_file = FLASH_FS.open(req->localFileName.c_str(), "r");
            }

            fbdo->_ss.fcs.fileSize = Signer.getCfg()->_int.fb_file.size();
        }
    }

    std::string header;

    if (req->requestType == fb_esp_fcs_request_type_upload)
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

    if (req->requestType == fb_esp_fcs_request_type_upload)
    {
        ut->appendP(header, fb_esp_pgm_str_8);
        header += req->mime;
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_12);
        char *tmp = ut->intStr(fbdo->_ss.fcs.fileSize);
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
    ut->appendP(header, fb_esp_pgm_str_36);
    ut->appendP(header, fb_esp_pgm_str_21);

    int ret = fbdo->httpClient.send(header.c_str(), "");

    if (ret == 0)
    {
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

        ret = handleResponse(fbdo);

        if (Signer.getCfg()->_int.fb_file && req->requestType == fb_esp_fcs_request_type_download)
            Signer.getCfg()->_int.fb_file.close();

        return ret;
    }

    if (Signer.getCfg()->_int.fb_file && req->requestType == fb_esp_fcs_request_type_download)
        Signer.getCfg()->_int.fb_file.close();

    return false;
}

bool FB_CloudStorage::handleResponse(FirebaseData *fbdo)
{

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->httpClient.stream();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    char *header = nullptr;
    char *payload = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->_ss.resp_size;
    int pBufPos = 0;
    int hBufPos = 0;
    int chunkBufSize = stream->available();
    int hstate = 0;
    int pstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int payloadRead = 0;
    size_t defaultChunkSize = fbdo->_ss.resp_size;
    struct fb_esp_auth_token_error_t error;
    error.code = -1;
    std::string js;
    FirebaseJson json;
    FirebaseJsonData data;
    bool jso = false;
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

    chunkBufSize = stream->available();

    dataTime = millis();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0 || payloadRead < response.contentLen)
        {
            if (!fbdo->reconnect(dataTime))
                return false;

            chunkBufSize = stream->available();

            if (chunkBufSize <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
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
                    else
                    {
                        //stream payload data
                        payload = ut->newS(payloadLen);
                        pstate = 1;
                        memcpy(payload, header, readLen);
                        pBufPos = readLen;
                        ut->delS(header);
                        hstate = 0;
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
                                    error.code = 0;
                                    if (available > defaultChunkSize)
                                        available = defaultChunkSize;

                                    size_t read = fbdo->httpClient.stream()->read(buf, available);
                                    if (read == available)
                                        Signer.getCfg()->_int.fb_file.write(buf, read);
                                    available = fbdo->httpClient.stream()->available();
                                }
                                delete[] buf;
                                break;
                            }
                            else
                            {

                                pChunkIdx++;

                                pChunk = ut->newS(chunkBufSize + 1);

                                if (!payload || pstate == 0)
                                {
                                    pstate = 1;
                                    payload = ut->newS(payloadLen + 1);
                                }

                                //read the avilable data
                                int readLen = 0;

                                //chunk transfer encoding?
                                if (response.isChunkedEnc)
                                    readLen = ut->readChunkedData(stream, pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                                else
                                {
                                    if (fbdo->_ss.fcs.requestType == fb_esp_fcs_request_type_list)
                                        readLen = ut->readLine(stream, pChunk, chunkBufSize);
                                    else
                                        readLen = stream->readBytes(pChunk, chunkBufSize);
                                }

                                if (readLen > 0)
                                {
                                    payloadRead += readLen;
                                    if (fbdo->_ss.fcs.requestType == fb_esp_fcs_request_type_list)
                                    {
                                        if (ut->strposP(pChunk, fb_esp_pgm_str_163, 0) > -1)
                                        {
                                            js.clear();
                                            jso = true;
                                            ut->appendP(js, fb_esp_pgm_str_163);
                                        }
                                        else if (ut->strposP(pChunk, fb_esp_pgm_str_127, 0) > -1)
                                        {
                                            if (jso)
                                            {
                                                fb_esp_fcs_file_list_item_t itm;
                                                ut->appendP(js, fb_esp_pgm_str_127);
                                                json.setJsonData(js.c_str());
                                                tmp = ut->strP(fb_esp_pgm_str_274);
                                                json.get(data, tmp);
                                                ut->delS(tmp);
                                                if (data.success)
                                                    itm.name = data.stringValue.c_str();
                                                tmp = ut->strP(fb_esp_pgm_str_275);
                                                json.get(data, tmp);
                                                ut->delS(tmp);
                                                if (data.success)
                                                    itm.bucket = data.stringValue.c_str();
                                                fbdo->_ss.fcs.files.items.push_back(itm);
                                            }
                                            jso = false;
                                            js.clear();
                                        }
                                        else
                                            js += pChunk;
                                    }

                                    if (pBufPos + readLen <= payloadLen)
                                        memcpy(payload + pBufPos, pChunk, readLen);
                                    else
                                    {
                                        //in case of the accumulated payload size is bigger than the char array
                                        //reallocate the char array
                                        char *buf = ut->newS(pBufPos + readLen + 1);
                                        memcpy(buf, payload, pBufPos);

                                        memcpy(buf + pBufPos, pChunk, readLen);

                                        payloadLen = pBufPos + readLen;
                                        ut->delS(payload);
                                        payload = ut->newS(payloadLen + 1);
                                        memcpy(payload, buf, payloadLen);
                                        ut->delS(buf);
                                    }
                                }

                                ut->delS(pChunk);
                                if (readLen < 0 && payloadRead >= response.contentLen)
                                    break;
                                if (readLen > 0)
                                    pBufPos += readLen;
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

        if (hstate == 1)
            ut->delS(header);

        //parse the payload
        if (payload)
        {
            if (payload[0] == '{' && payload[strlen(payload) - 1] == '}')
            {
                FirebaseJson json;
                FirebaseJsonData data;
                json.setJsonData(payload);

                char *tmp = ut->strP(fb_esp_pgm_str_257);
                json.get(data, tmp);
                ut->delS(tmp);

                if (data.success)
                {
                    error.code = data.intValue;
                    tmp = ut->strP(fb_esp_pgm_str_258);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.error = data.stringValue.c_str();
                }
                else
                {
                    error.code = 0;
                    tmp = ut->strP(fb_esp_pgm_str_274);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.name = data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_275);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.bucket = data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_276);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.generation = atof(data.stringValue.c_str());

                    tmp = ut->strP(fb_esp_pgm_str_277);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.contentType = data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_278);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.size = atoi(data.stringValue.c_str());

                    tmp = ut->strP(fb_esp_pgm_str_279);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.etag = data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_280);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.crc32 = data.stringValue.c_str();

                    tmp = ut->strP(fb_esp_pgm_str_272);
                    json.get(data, tmp);
                    ut->delS(tmp);
                    if (data.success)
                        fbdo->_ss.fcs.meta.downloadTokens = data.stringValue.c_str();
                }
            }

            fbdo->_ss.content_length = response.payloadLen;

            fbdo->_ss.json.clear();
            fbdo->_ss.arr.clear();
        }

        if (pstate == 1)
            ut->delS(payload);

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