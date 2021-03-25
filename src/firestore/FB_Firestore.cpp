/**
 * Google's Cloud Firestore class, Forestore.cpp version 1.0.5
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

#ifndef _FB_FIRESTORE_CPP_
#define _FB_FIRESTORE_CPP_
#include "FB_Firestore.h"

FB_Firestore::FB_Firestore()
{
}

FB_Firestore::~FB_Firestore()
{
}

bool FB_Firestore::exportDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_export_docs;
    req.projectId = projectId;
    req.databaseId = databaseId;

    std::string outputUriPrefix;

    fbdo->_ss.json.clear();

    ut->appendP(outputUriPrefix, fb_esp_pgm_str_350);
    outputUriPrefix += bucketID;
    ut->appendP(outputUriPrefix, fb_esp_pgm_str_1);
    outputUriPrefix += storagePath;

    char *tmp = ut->strP(fb_esp_pgm_str_347);
    fbdo->_ss.json.add(tmp, outputUriPrefix.c_str());
    ut->delS(tmp);

    tmp = ut->strP(fb_esp_pgm_str_346);
    fbdo->_ss.arr.clear();

    if (strlen(collectionIds) > 0)
    {
        std::vector<std::string> colIds = std::vector<std::string>();
        ut->splitTk(collectionIds, colIds, ",");
        for (size_t i = 0; i < colIds.size(); i++)
            fbdo->_ss.arr.add(colIds[i].c_str());
    }

    fbdo->_ss.json.add(tmp, fbdo->_ss.arr);
    ut->delS(tmp);

    fbdo->_ss.json.int_tostr(req.payload);

    return sendRequest(fbdo, &req);
}

bool FB_Firestore::importDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_import_docs;
    req.projectId = projectId;
    req.databaseId = databaseId;

    std::string inputUriPrefix;

    fbdo->_ss.json.clear();

    ut->appendP(inputUriPrefix, fb_esp_pgm_str_350);
    inputUriPrefix += bucketID;
    ut->appendP(inputUriPrefix, fb_esp_pgm_str_1);
    inputUriPrefix += storagePath;

    char *tmp = ut->strP(fb_esp_pgm_str_348);
    fbdo->_ss.json.add(tmp, inputUriPrefix.c_str());
    ut->delS(tmp);

    tmp = ut->strP(fb_esp_pgm_str_346);
    fbdo->_ss.arr.clear();

    if (strlen(collectionIds) > 0)
    {
        std::vector<std::string> colIds = std::vector<std::string>();
        ut->splitTk(collectionIds, colIds, ",");
        for (size_t i = 0; i < colIds.size(); i++)
            fbdo->_ss.arr.add(colIds[i].c_str());
    }

    fbdo->_ss.json.add(tmp, fbdo->_ss.arr);
    ut->delS(tmp);

    fbdo->_ss.json.int_tostr(req.payload);

    return sendRequest(fbdo, &req);
}

bool FB_Firestore::createDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *mask)
{
    size_t count = 0;
    std::string collectionId, documentId;
    collectionId = documentPath;
    size_t p = collectionId.find_last_of("/");
    for (size_t i = 0; i < strlen(documentPath); i++)
    {
        if (documentPath[i] == '/')
            count++;
    }

    if (p != std::string::npos && count % 2 > 0)
    {
        documentId = collectionId.substr(p + 1, collectionId.length() - p - 1);
        collectionId = collectionId.substr(0, p);
    }

    return createDocument(fbdo, projectId, databaseId, collectionId.c_str(), documentId.c_str(), content, mask);
}

bool FB_Firestore::createDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, const char *documentId, const char *content, const char *mask)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_create_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentId = documentId;
    req.collectionId = collectionId;
    req.payload = content;
    req.mask = mask;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::patchDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *updateMask, const char *mask, const char *exists, const char *updateTime)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_patch_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;
    req.payload = content;
    req.updateMask = updateMask;
    req.mask = mask;
    req.exists = exists;
    req.updateTime = updateTime;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::getDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *mask, const char *transaction, const char *readTime)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_get_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;
    req.mask = mask;
    req.transaction = transaction;
    req.readTime = readTime;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::runQuery(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, FirebaseJson *structuredQuery, fb_esp_firestore_consistency_mode consistencyMode, const char *consistency)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_run_query;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;
    char *tmp = nullptr;
    fbdo->_ss.json.clear();
    if (consistencyMode != fb_esp_firestore_consistency_mode_undefined)
    {
        if (consistencyMode != fb_esp_firestore_consistency_mode_transaction)
            tmp = ut->strP(fb_esp_pgm_str_537);
        else if (consistencyMode != fb_esp_firestore_consistency_mode_newTransaction)
            tmp = ut->strP(fb_esp_pgm_str_538);
        else if (consistencyMode != fb_esp_firestore_consistency_mode_readTime)
            tmp = ut->strP(fb_esp_pgm_str_539);
        fbdo->_ss.json.add((const char *)tmp, consistency);
        ut->delS(tmp);
    }

    tmp = ut->strP(fb_esp_pgm_str_536);
    if (structuredQuery)
        fbdo->_ss.json.add((const char *)tmp, *structuredQuery);
    else
    {
        static FirebaseJson js;
        fbdo->_ss.json.add((const char *)tmp, js);
    }
    ut->delS(tmp);

    fbdo->_ss.json.int_tostr(req.payload);
    fbdo->_ss.json.clear();

    return sendRequest(fbdo, &req);
}

bool FB_Firestore::deleteDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *exists, const char *updateTime)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_delete_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;
    req.exists = exists;
    req.updateTime = updateTime;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::listDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, int pageSize, const char *pageToken, const char *orderBy, const char *mask, bool showMissing)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_list_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.collectionId = collectionId;
    req.pageSize = pageSize;
    req.pageToken = pageToken;
    req.orderBy = orderBy;
    req.mask = mask;
    req.showMissing = showMissing;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::listCollectionIds(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, int pageSize, const char *pageToken)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_list_collection;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;

    fbdo->_ss.json.clear();
    char *tmp = ut->strP(fb_esp_pgm_str_357);
    fbdo->_ss.json.add(tmp, pageSize);
    ut->delS(tmp);
    tmp = ut->strP(fb_esp_pgm_str_358);
    fbdo->_ss.json.add(tmp, pageToken);
    ut->delS(tmp);

    fbdo->_ss.json.int_tostr(req.payload);

    return sendRequest(fbdo, &req);
}

void FB_Firestore::begin(UtilsClass *u)
{
    ut = u;
}

bool FB_Firestore::sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
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

    fbdo->_ss.cfs.payload.clear();

    connect(fbdo);

    return firestore_sendRequest(fbdo, req);
}

bool FB_Firestore::firestore_sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
{
    std::string token = Signer.getToken(Signer.getTokenType());

    std::string header;
    if (req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_list_doc)
        ut->appendP(header, fb_esp_pgm_str_25);
    else if (req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_export_docs || req->requestType == fb_esp_firestore_request_type_import_docs || req->requestType == fb_esp_firestore_request_type_create_doc)
        ut->appendP(header, fb_esp_pgm_str_24);
    else if (req->requestType == fb_esp_firestore_request_type_patch_doc)
        ut->appendP(header, fb_esp_pgm_str_26);
    else if (req->requestType == fb_esp_firestore_request_type_delete_doc)
        ut->appendP(header, fb_esp_pgm_str_27);

    ut->appendP(header, fb_esp_pgm_str_6);
    ut->appendP(header, fb_esp_pgm_str_326);

    if (req->projectId.length() == 0)
        header += Signer.getCfg()->service_account.data.project_id;
    else
        header += req->projectId;

    ut->appendP(header, fb_esp_pgm_str_341);
    if (req->databaseId.length() > 0)
        header += req->databaseId;
    else
        ut->appendP(header, fb_esp_pgm_str_342);

    if (req->requestType == fb_esp_firestore_request_type_export_docs)
        ut->appendP(header, fb_esp_pgm_str_344);
    else if (req->requestType == fb_esp_firestore_request_type_import_docs)
        ut->appendP(header, fb_esp_pgm_str_345);
    else if (req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_list_doc || req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_create_doc || req->requestType == fb_esp_firestore_request_type_patch_doc || req->requestType == fb_esp_firestore_request_type_delete_doc)
    {
        ut->appendP(header, fb_esp_pgm_str_351);

        bool hasParam = false;
        if (req->requestType == fb_esp_firestore_request_type_create_doc)
        {
            if (req->collectionId.length() > 0)
            {
                if (req->collectionId[0] != '/')
                    ut->appendP(header, fb_esp_pgm_str_1);
            }
            else
                ut->appendP(header, fb_esp_pgm_str_1);

            header += req->collectionId;
            ut->appendP(header, fb_esp_pgm_str_343);
            header += req->documentId;
            hasParam = true;
        }
        else if (req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_patch_doc || req->requestType == fb_esp_firestore_request_type_delete_doc)
        {
            if (req->documentPath.length() > 0)
            {
                if (req->documentPath[0] != '/')
                    ut->appendP(header, fb_esp_pgm_str_1);
            }
            else
                ut->appendP(header, fb_esp_pgm_str_1);

            header += req->documentPath;
            if (req->requestType == fb_esp_firestore_request_type_list_collection)
                ut->appendP(header, fb_esp_pgm_str_362);
            else if (req->requestType == fb_esp_firestore_request_type_run_query)
                ut->appendP(header, fb_esp_pgm_str_535);
        }
        else if (req->requestType == fb_esp_firestore_request_type_list_doc)
        {
            if (req->collectionId.length() > 0)
            {
                if (req->collectionId[0] != '/')
                    ut->appendP(header, fb_esp_pgm_str_1);
            }
            else
                ut->appendP(header, fb_esp_pgm_str_1);

            header += req->collectionId;
            ut->appendP(header, fb_esp_pgm_str_173);
            ut->appendP(header, fb_esp_pgm_str_357);
            ut->appendP(header, fb_esp_pgm_str_361);
            char *tmp = ut->intStr(req->pageSize);
            header += tmp;
            ut->delS(tmp);
            ut->appendP(header, fb_esp_pgm_str_172);
            ut->appendP(header, fb_esp_pgm_str_358);
            ut->appendP(header, fb_esp_pgm_str_361);
            header += req->pageToken;
            ut->appendP(header, fb_esp_pgm_str_172);
            ut->appendP(header, fb_esp_pgm_str_359);
            header += req->orderBy;
            ut->appendP(header, fb_esp_pgm_str_172);
            ut->appendP(header, fb_esp_pgm_str_360);
            tmp = ut->boolStr(req->showMissing);
            header += tmp;
            ut->delS(tmp);
            hasParam = true;
        }

        if (req->requestType == fb_esp_firestore_request_type_patch_doc)
        {
            if (req->updateMask.length() > 0)
            {
                std::vector<std::string> docMasks = std::vector<std::string>();
                ut->splitTk(req->updateMask, docMasks, ",");
                for (size_t i = 0; i < docMasks.size(); i++)
                {
                    if (!hasParam)
                        ut->appendP(header, fb_esp_pgm_str_173);
                    else
                        ut->appendP(header, fb_esp_pgm_str_172);
                    ut->appendP(header, fb_esp_pgm_str_352);
                    header += docMasks[i];
                    hasParam = true;
                }
            }
        }

        if (req->mask.length() > 0)
        {
            std::vector<std::string> docMasks = std::vector<std::string>();
            ut->splitTk(req->mask, docMasks, ",");
            for (size_t i = 0; i < docMasks.size(); i++)
            {
                if (!hasParam)
                    ut->appendP(header, fb_esp_pgm_str_173);
                else
                    ut->appendP(header, fb_esp_pgm_str_172);

                ut->appendP(header, fb_esp_pgm_str_349);
                header += docMasks[i];
                hasParam = true;
            }
        }

        if (req->requestType == fb_esp_firestore_request_type_get_doc)
        {
            if (req->transaction.length() > 0)
            {
                if (!hasParam)
                    ut->appendP(header, fb_esp_pgm_str_173);
                else
                    ut->appendP(header, fb_esp_pgm_str_172);
                ut->appendP(header, fb_esp_pgm_str_355);
                header += req->transaction;
                hasParam = true;
            }

            if (req->readTime.length() > 0)
            {
                if (!hasParam)
                    ut->appendP(header, fb_esp_pgm_str_173);
                else
                    ut->appendP(header, fb_esp_pgm_str_172);
                ut->appendP(header, fb_esp_pgm_str_356);
                header += req->readTime;
            }
        }
        else if (req->requestType == fb_esp_firestore_request_type_patch_doc || req->requestType == fb_esp_firestore_request_type_delete_doc)
        {
            if (req->exists.length() > 0)
            {
                if (!hasParam)
                    ut->appendP(header, fb_esp_pgm_str_173);
                else
                    ut->appendP(header, fb_esp_pgm_str_172);
                ut->appendP(header, fb_esp_pgm_str_353);
                header += req->exists;
                hasParam = true;
            }
            if (req->updateTime.length() > 0)
            {
                if (!hasParam)
                    ut->appendP(header, fb_esp_pgm_str_173);
                else
                    ut->appendP(header, fb_esp_pgm_str_172);
                ut->appendP(header, fb_esp_pgm_str_354);
                header += req->updateTime;
            }
        }
    }

    ut->appendP(header, fb_esp_pgm_str_30);

    if (req->payload.length() > 0)
    {
        ut->appendP(header, fb_esp_pgm_str_8);
        ut->appendP(header, fb_esp_pgm_str_129);
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_12);
        char *tmp = ut->intStr(req->payload.length());
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);
    }

    ut->appendP(header, fb_esp_pgm_str_31);
    ut->appendP(header, fb_esp_pgm_str_340);
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

    fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;

    int ret = fbdo->httpClient.send(header.c_str(), req->payload.c_str());
    header.clear();
    req->payload.clear();
    std::string().swap(header);
    std::string().swap(req->payload);

    if (ret == 0)
    {
        fbdo->_ss.connected = true;
        if (handleResponse(fbdo))
        {
            Signer.getCfg()->_int.fb_processing = false;
            return true;
        }
    }
    else
        fbdo->_ss.connected = false;

    Signer.getCfg()->_int.fb_processing = false;

    return false;
}

void FB_Firestore::rescon(FirebaseData *fbdo, const char *host)
{
    if (!fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout ||  fbdo->_ss.con_mode != fb_esp_con_mode_firestore || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->_ss.host = host;
    fbdo->_ss.con_mode = fb_esp_con_mode_firestore;
}

bool FB_Firestore::connect(FirebaseData *fbdo)
{
    std::string host;
    ut->appendP(host, fb_esp_pgm_str_340);
    ut->appendP(host, fb_esp_pgm_str_120);
    rescon(fbdo, host.c_str());
    fbdo->httpClient.begin(host.c_str(), 443);
    return true;
}

bool FB_Firestore::handleResponse(FirebaseData *fbdo)
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
    fbdo->_ss.cfs.payload.clear();

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
                                if (payloadRead < payloadLen)
                                    fbdo->_ss.cfs.payload += pChunk;
                            }

                            ut->delS(pChunk);

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
            ut->delS(header);

        //parse the payload
        if (fbdo->_ss.cfs.payload.length() > 0)
        {
            if (fbdo->_ss.cfs.payload[0] == '{')
            {
                fbdo->_ss.json.setJsonData(fbdo->_ss.cfs.payload.c_str());
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
                    fbdo->_ss.cfs.payload.clear();
                }
                else
                {
                    error.code = 0;
                }
            }
            //JSON Array payload
            else if (fbdo->_ss.cfs.payload[0] == '[')
                error.code = 0;

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