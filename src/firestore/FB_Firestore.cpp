/**
 * Google's Cloud Firestore class, Forestore.cpp version 1.1.4
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created September 3, 2021
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

#ifdef ENABLE_FIRESTORE

#ifndef _FB_FIRESTORE_CPP_
#define _FB_FIRESTORE_CPP_
#include "FB_Firestore.h"

FB_Firestore::FB_Firestore()
{
}

FB_Firestore::~FB_Firestore()
{
}

bool FB_Firestore::mExportDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_export_docs;
    req.projectId = projectId;
    req.databaseId = databaseId;

    std::string outputUriPrefix;

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    if (!fbdo->_ss.arrPtr)
        fbdo->_ss.arrPtr = new FirebaseJsonArray();

    fbdo->_ss.jsonPtr->clear();
    fbdo->_ss.arrPtr->clear();

    ut->appendP(outputUriPrefix, fb_esp_pgm_str_350);
    outputUriPrefix += bucketID;
    ut->appendP(outputUriPrefix, fb_esp_pgm_str_1);
    outputUriPrefix += storagePath;

    char *tmp = ut->strP(fb_esp_pgm_str_347);
    fbdo->_ss.jsonPtr->add((const char *)tmp, outputUriPrefix.c_str());
    ut->delS(tmp);

    if (strlen(collectionIds) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_346);

        std::vector<std::string> colIds = std::vector<std::string>();
        ut->splitTk(collectionIds, colIds, ",");
        for (size_t i = 0; i < colIds.size(); i++)
            fbdo->_ss.arrPtr->add(colIds[i].c_str());

        fbdo->_ss.jsonPtr->add(tmp, *fbdo->_ss.arrPtr);
        ut->delS(tmp);
    }

    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();
    fbdo->_ss.arrPtr->clear();

    bool ret = sendRequest(fbdo, &req);
    ut->clearS(req.payload);
    return ret;
}

bool FB_Firestore::mImportDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_import_docs;
    req.projectId = projectId;
    req.databaseId = databaseId;

    std::string inputUriPrefix;

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    if (!fbdo->_ss.arrPtr)
        fbdo->_ss.arrPtr = new FirebaseJsonArray();

    fbdo->_ss.jsonPtr->clear();
    fbdo->_ss.arrPtr->clear();

    ut->appendP(inputUriPrefix, fb_esp_pgm_str_350);
    inputUriPrefix += bucketID;
    ut->appendP(inputUriPrefix, fb_esp_pgm_str_1);
    inputUriPrefix += storagePath;

    char *tmp = ut->strP(fb_esp_pgm_str_348);
    fbdo->_ss.jsonPtr->add(tmp, inputUriPrefix.c_str());
    ut->delS(tmp);

    if (strlen(collectionIds) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_346);
        std::vector<std::string> colIds = std::vector<std::string>();
        ut->splitTk(collectionIds, colIds, ",");
        for (size_t i = 0; i < colIds.size(); i++)
            fbdo->_ss.arrPtr->add(colIds[i].c_str());

        fbdo->_ss.jsonPtr->add(tmp, *fbdo->_ss.arrPtr);
        ut->delS(tmp);
    }

    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();
    fbdo->_ss.arrPtr->clear();

    bool ret = sendRequest(fbdo, &req);
    ut->clearS(req.payload);
    return ret;
}

bool FB_Firestore::mCreateDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *mask)
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

bool FB_Firestore::mCreateDocument2(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, const char *documentId, const char *content, const char *mask)
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

bool FB_Firestore::mPatchDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *updateMask, const char *mask, const char *exists, const char *updateTime)
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

bool FB_Firestore::setFieldTransform(FirebaseJson *json, struct fb_esp_firestore_document_write_field_transforms_t *field_transforms)
{
    std::string path;
    bool hasTransform = false;
    json->clear();

    if (strlen(field_transforms->transform_content) > 0)
    {
        if (strlen(field_transforms->fieldPath) > 0)
        {
            ut->appendP(path, fb_esp_pgm_str_557, true);
            json->add(path.c_str(), field_transforms->fieldPath);
            hasTransform = true;
        }

        if (field_transforms->transform_type == fb_esp_firestore_transform_type_set_to_server_value)
        {
            ut->appendP(path, fb_esp_pgm_str_558, true);
            json->add(path.c_str(), field_transforms->transform_content);
            hasTransform = true;
        }
        else if (field_transforms->transform_type != fb_esp_firestore_transform_type_undefined)
        {
            FirebaseJson js;

            if (field_transforms->transform_type == fb_esp_firestore_transform_type_increment)
                ut->appendP(path, fb_esp_pgm_str_559, true);
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_maaximum)
                ut->appendP(path, fb_esp_pgm_str_560, true);
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_minimum)
                ut->appendP(path, fb_esp_pgm_str_561, true);
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_append_missing_elements)
                ut->appendP(path, fb_esp_pgm_str_562, true);
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_remove_all_from_array)
                ut->appendP(path, fb_esp_pgm_str_563, true);

            js.clear();
            js.setJsonData(field_transforms->transform_content);
            json->add(path.c_str(), js);
            js.clear();
            hasTransform = true;
        }
    }

    return hasTransform;
}

bool FB_Firestore::mCommitDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, std::vector<struct fb_esp_firestore_document_write_t> writes, const char *transaction, bool async)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_commit_document;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.transaction = transaction;
    req.async = async;

    if (writes.size() > 0)
    {
        std::string path, updateMaskPath, docPathBase, docPath;
        ut->appendP(updateMaskPath, fb_esp_pgm_str_400);
        ut->appendP(updateMaskPath, fb_esp_pgm_str_1);
        ut->appendP(updateMaskPath, fb_esp_pgm_str_556);

        char *tmp = nullptr;
        char *tmp2 = nullptr;
        FirebaseJson json;
        bool hasCurDoc = false;

        std::string writesArr[writes.size()];

        if (!fbdo->_ss.jsonPtr)
            fbdo->_ss.jsonPtr = new FirebaseJson();

        if (!fbdo->_ss.arrPtr)
            fbdo->_ss.arrPtr = new FirebaseJsonArray();

        fbdo->_ss.jsonPtr->clear();
        fbdo->_ss.arrPtr->clear();

        ut->appendP(docPathBase, fb_esp_pgm_str_395, true);

        if (req.projectId.length() == 0)
            docPathBase += Signer.getCfg()->service_account.data.project_id;
        else
            docPathBase += req.projectId;

        ut->appendP(docPathBase, fb_esp_pgm_str_341);
        if (req.databaseId.length() > 0)
            docPathBase += req.databaseId;
        else
            ut->appendP(docPathBase, fb_esp_pgm_str_342);

        ut->appendP(docPathBase, fb_esp_pgm_str_351);

        for (size_t i = 0; i < writes.size(); i++)
        {
            struct fb_esp_firestore_document_write_t *write = &writes[i];

            json.clear();

            hasCurDoc = false;

            if (strlen(write->update_masks) > 0)
            {
                std::vector<std::string> masks = std::vector<std::string>();
                ut->splitTk(write->update_masks, masks, ",");
                for (size_t j = 0; j < masks.size(); j++)
                    fbdo->_ss.arrPtr->add(masks[j].c_str());

                fbdo->_ss.jsonPtr->set(updateMaskPath.c_str(), *fbdo->_ss.arrPtr);
                fbdo->_ss.arrPtr->clear();
            }

            if (setFieldTransform(&json, &write->update_transforms))
            {
                ut->appendP(path, fb_esp_pgm_str_567, true);
                fbdo->_ss.jsonPtr->add(path.c_str(), json);
                json.clear();
            }

            if (strlen(write->current_document.exists) > 0 || strlen(write->current_document.update_time) > 0)
            {

                if (strlen(write->current_document.exists) > 0)
                {
                    tmp2 = ut->strP(fb_esp_pgm_str_569);
                    tmp = ut->strP(fb_esp_pgm_str_107);
                    if (strcmp(write->current_document.exists, tmp) == 0)
                    {
                        json.add((const char *)tmp2, (const char *)tmp);
                        hasCurDoc = true;
                    }
                    else
                    {
                        ut->delS(tmp);
                        tmp = ut->strP(fb_esp_pgm_str_106);
                        if (strcmp(write->current_document.exists, tmp) == 0)
                        {
                            json.add((const char *)tmp2, (const char *)tmp);
                            hasCurDoc = true;
                        }
                    }
                    ut->delS(tmp);
                    ut->delS(tmp2);
                }
                else if (strlen(write->current_document.update_time) > 0 && !hasCurDoc)
                {
                    tmp = ut->strP(fb_esp_pgm_str_467);
                    json.add((const char *)tmp, write->current_document.update_time);
                    hasCurDoc = true;
                }

                if (hasCurDoc)
                {
                    ut->appendP(path, fb_esp_pgm_str_566, true);
                    fbdo->_ss.jsonPtr->add(path.c_str(), json);
                    json.clear();
                }
            }

            if (strlen(write->update_document_content) > 0 && strlen(write->update_document_path) > 0 && write->type == fb_esp_firestore_document_write_type_update)
            {
                docPath = docPathBase;

                if (write->update_document_path[0] != '/')
                    ut->appendP(docPath, fb_esp_pgm_str_1);

                docPath += write->update_document_path;

                ut->appendP(path, fb_esp_pgm_str_274, true);
                json.setJsonData(write->update_document_content);
                json.add(path.c_str(), docPath.c_str());

                ut->appendP(path, fb_esp_pgm_str_118, true);
                fbdo->_ss.jsonPtr->add(path.c_str(), json);
                json.clear();
            }
            else if (strlen(write->delete_document_path) > 0 && write->type == fb_esp_firestore_document_write_type_delete)
            {
                docPath = docPathBase;

                if (write->delete_document_path[0] != '/')
                    ut->appendP(docPath, fb_esp_pgm_str_1);

                docPath += write->delete_document_path;

                ut->appendP(path, fb_esp_pgm_str_119, true);
                fbdo->_ss.jsonPtr->add(path.c_str(), docPath.c_str());
            }
            else if (strlen(write->document_transform.transform_document_path) > 0 && write->document_transform.field_transforms.size() > 0 && write->type == fb_esp_firestore_document_write_type_transform)
            {
                fbdo->_ss.arrPtr->clear();

                for (size_t j = 0; j < write->document_transform.field_transforms.size(); j++)
                {
                    json.clear();
                    if (setFieldTransform(&json, &write->document_transform.field_transforms[j]))
                        fbdo->_ss.arrPtr->add(json);
                }

                json.clear();

                docPath = docPathBase;

                if (write->document_transform.transform_document_path[0] != '/')
                    ut->appendP(docPath, fb_esp_pgm_str_1);

                docPath += write->document_transform.transform_document_path;

                ut->appendP(path, fb_esp_pgm_str_564, true);
                json.add(path.c_str(), docPath.c_str());

                ut->appendP(path, fb_esp_pgm_str_565, true);
                json.add(path.c_str(), *fbdo->_ss.arrPtr);
                fbdo->_ss.arrPtr->clear();

                ut->appendP(path, fb_esp_pgm_str_568, true);
                fbdo->_ss.jsonPtr->add(path.c_str(), json);
                json.clear();
            }

            writesArr[i] = fbdo->_ss.jsonPtr->raw();
            fbdo->_ss.jsonPtr->clear();
            fbdo->_ss.arrPtr->clear();
        }

        for (size_t i = 0; i < writes.size(); i++)
        {
            fbdo->_ss.jsonPtr->setJsonData(writesArr[i].c_str());
            fbdo->_ss.arrPtr->add(*fbdo->_ss.jsonPtr);
            fbdo->_ss.jsonPtr->clear();
            std::string().swap(writesArr[i]);
        }

        tmp = ut->strP(fb_esp_pgm_str_555);
        fbdo->_ss.jsonPtr->add((const char *)tmp, *fbdo->_ss.arrPtr);
        ut->delS(tmp);
        fbdo->_ss.arrPtr->clear();

        if (strlen(transaction))
        {
            tmp = ut->strP(fb_esp_pgm_str_537);
            fbdo->_ss.jsonPtr->add((const char *)tmp, transaction);
            ut->delS(tmp);
        }

        req.payload = fbdo->_ss.jsonPtr->raw();
        fbdo->_ss.jsonPtr->clear();
    }

    bool ret = sendRequest(fbdo, &req);
    ut->clearS(req.payload);
    return ret;
}

bool FB_Firestore::mGetDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *mask, const char *transaction, const char *readTime)
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

bool FB_Firestore::mBeginTransaction(FirebaseData *fbdo, const char *projectId, const char *databaseId, TransactionOptions *transactionOptions)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_begin_transaction;
    req.projectId = projectId;
    req.databaseId = databaseId;

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();

    if (transactionOptions)
    {
        if (strlen(transactionOptions->readOnly.readTime) > 0)
        {
            char *tmp = ut->strP(fb_esp_pgm_str_571);
            fbdo->_ss.jsonPtr->set((const char *)tmp, transactionOptions->readOnly.readTime);
            ut->delS(tmp);
        }
        else if (strlen(transactionOptions->readWrite.retryTransaction) > 0)
        {
            char *tmp = ut->strP(fb_esp_pgm_str_572);
            fbdo->_ss.jsonPtr->set((const char *)tmp, transactionOptions->readWrite.retryTransaction);
            ut->delS(tmp);
        }
    }

    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();
    bool ret = sendRequest(fbdo, &req);
    ut->clearS(req.payload);
    return ret;
}

bool FB_Firestore::mRollback(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *transaction)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_rollback;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.async = false;

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();

    if (strlen(transaction) > 0)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_537);
        fbdo->_ss.jsonPtr->add((const char *)tmp, transaction);
        ut->delS(tmp);
    }
    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();
    bool ret = sendRequest(fbdo, &req);
    ut->clearS(req.payload);
    return ret;
}

bool FB_Firestore::mRunQuery(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, FirebaseJson *structuredQuery, fb_esp_firestore_consistency_mode consistencyMode, const char *consistency)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_run_query;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;
    char *tmp = nullptr;
    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();
    if (consistencyMode != fb_esp_firestore_consistency_mode_undefined)
    {
        if (consistencyMode != fb_esp_firestore_consistency_mode_transaction)
            tmp = ut->strP(fb_esp_pgm_str_537);
        else if (consistencyMode != fb_esp_firestore_consistency_mode_newTransaction)
            tmp = ut->strP(fb_esp_pgm_str_538);
        else if (consistencyMode != fb_esp_firestore_consistency_mode_readTime)
            tmp = ut->strP(fb_esp_pgm_str_539);
        fbdo->_ss.jsonPtr->add((const char *)tmp, consistency);
        ut->delS(tmp);
    }

    tmp = ut->strP(fb_esp_pgm_str_536);
    if (structuredQuery)
        fbdo->_ss.jsonPtr->add((const char *)tmp, *structuredQuery);
    else
    {
        static FirebaseJson js;
        fbdo->_ss.jsonPtr->add((const char *)tmp, js);
    }
    ut->delS(tmp);

    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();

    bool ret = sendRequest(fbdo, &req);
    ut->clearS(req.payload);
    return ret;
}

bool FB_Firestore::mDeleteDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *exists, const char *updateTime)
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

bool FB_Firestore::mListDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, const char* pageSize, const char *pageToken, const char *orderBy, const char *mask, bool showMissing)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_list_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.collectionId = collectionId;
    req.pageSize = atoi(pageSize);
    req.pageToken = pageToken;
    req.orderBy = orderBy;
    req.mask = mask;
    req.showMissing = showMissing;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mListCollectionIds(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char* pageSize, const char *pageToken)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_list_collection;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;

    if (!fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr = new FirebaseJson();

    fbdo->_ss.jsonPtr->clear();
    char *tmp = ut->strP(fb_esp_pgm_str_357);
    fbdo->_ss.jsonPtr->add(tmp, atoi(pageSize));
    ut->delS(tmp);
    tmp = ut->strP(fb_esp_pgm_str_358);
    fbdo->_ss.jsonPtr->add(tmp, pageToken);
    ut->delS(tmp);

    req.payload = fbdo->_ss.jsonPtr->raw();
    fbdo->_ss.jsonPtr->clear();
    bool ret = sendRequest(fbdo, &req);
    ut->clearS(req.payload);
    return ret;
}

void FB_Firestore::begin(UtilsClass *u)
{
    ut = u;
}

bool FB_Firestore::sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
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

    if (fbdo->_ss.long_running_task > 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Signer.getCfg()->_int.fb_processing)
        return false;

    Signer.getCfg()->_int.fb_processing = true;

    fbdo->_ss.cfs.payload.clear();

    //close session if async mode changes
    if (fbdo->_ss.cfs.async && !req->async)
        fbdo->_ss.last_conn_ms = 0;

    connect(fbdo);

    return firestore_sendRequest(fbdo, req);
}

bool FB_Firestore::firestore_sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
{
    int ret = -1;
    std::string header;
    if (req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_list_doc)
        ut->appendP(header, fb_esp_pgm_str_25);
    else if (req->requestType == fb_esp_firestore_request_type_rollback || req->requestType == fb_esp_firestore_request_type_begin_transaction || req->requestType == fb_esp_firestore_request_type_commit_document || req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_export_docs || req->requestType == fb_esp_firestore_request_type_import_docs || req->requestType == fb_esp_firestore_request_type_create_doc)
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
    else if (req->requestType == fb_esp_firestore_request_type_begin_transaction)
    {
        ut->appendP(header, fb_esp_pgm_str_351);
        ut->appendP(header, fb_esp_pgm_str_573);
    }
    else if (req->requestType == fb_esp_firestore_request_type_rollback)
    {
        ut->appendP(header, fb_esp_pgm_str_351);
        ut->appendP(header, fb_esp_pgm_str_574);
    }
    else if (req->requestType == fb_esp_firestore_request_type_commit_document || req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_list_doc || req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_create_doc || req->requestType == fb_esp_firestore_request_type_patch_doc || req->requestType == fb_esp_firestore_request_type_delete_doc)
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
            header += NUM2S(req->pageSize).get();
            ut->appendP(header, fb_esp_pgm_str_172);
            ut->appendP(header, fb_esp_pgm_str_358);
            ut->appendP(header, fb_esp_pgm_str_361);
            header += req->pageToken;
            ut->appendP(header, fb_esp_pgm_str_172);
            ut->appendP(header, fb_esp_pgm_str_359);
            header += req->orderBy;
            ut->appendP(header, fb_esp_pgm_str_172);
            ut->appendP(header, fb_esp_pgm_str_360);
            header += NUM2S(req->showMissing).get();
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
        else if (req->requestType == fb_esp_firestore_request_type_commit_document)
        {
            ut->appendP(header, fb_esp_pgm_str_554);
            fbdo->_ss.cfs.async = req->async;
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
        header += NUM2S(req->payload.length()).get();
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

    ret = fbdo->tcpSend(header.c_str());
    ut->clearS(header);

    if (ret < 0)
        return false;

    ret = fbdo->tcpSend(Signer.getToken());

    if (ret < 0)
        return false;

    ut->appendP(header, fb_esp_pgm_str_21);

    ut->appendP(header, fb_esp_pgm_str_32);
    ut->appendP(header, fb_esp_pgm_str_36);
    ut->appendP(header, fb_esp_pgm_str_21);

    fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    ret = fbdo->tcpSend(header.c_str());
    if (ret == 0 && req->payload.length() > 0)
        ret = fbdo->tcpSend(req->payload.c_str());

    ut->clearS(header);
    ut->clearS(req->payload);

    if (ret == 0)
    {
        fbdo->_ss.connected = true;
        if (fbdo->_ss.cfs.async || handleResponse(fbdo))
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
    if (fbdo->_ss.cert_updated || !fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.con_mode != fb_esp_con_mode_firestore || strcmp(host, fbdo->_ss.host.c_str()) != 0)
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
    fbdo->tcpClient.begin(host.c_str(), 443);
    fbdo->_ss.max_payload_length = 0;
    return true;
}

bool FB_Firestore::handleResponse(FirebaseData *fbdo)
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
    fbdo->_ss.payload_length = 0;
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    defaultChunkSize = 2048;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = stream->available();
        ut->idle();
    }

    int availablePayload = chunkBufSize;

    dataTime = millis();

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
                    ut->idle();
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
                    ut->idle();
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

                            if (response.httpCode == 401)
                                Signer.authenticated = false;
                            else if (response.httpCode < 300)
                                Signer.authenticated = true;

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
                                fbdo->_ss.payload_length += availablePayload;
                                if (fbdo->_ss.max_payload_length < fbdo->_ss.payload_length)
                                    fbdo->_ss.max_payload_length = fbdo->_ss.payload_length;
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
                if (!fbdo->_ss.jsonPtr)
                    fbdo->_ss.jsonPtr = new FirebaseJson();
                if (!fbdo->_ss.dataPtr)
                    fbdo->_ss.dataPtr = new FirebaseJsonData();

                fbdo->_ss.jsonPtr->setJsonData(fbdo->_ss.cfs.payload.c_str());
                char *tmp = ut->strP(fb_esp_pgm_str_257);
                fbdo->_ss.jsonPtr->get(*fbdo->_ss.dataPtr, tmp);
                ut->delS(tmp);
                if (fbdo->_ss.dataPtr->success)
                {
                    error.code = fbdo->_ss.dataPtr->to<int>();
                    tmp = ut->strP(fb_esp_pgm_str_258);
                    fbdo->_ss.jsonPtr->get(*fbdo->_ss.dataPtr, tmp);
                    ut->delS(tmp);
                    if (fbdo->_ss.dataPtr->success)
                        fbdo->_ss.error = fbdo->_ss.dataPtr->to<const char *>();
                    fbdo->_ss.cfs.payload.clear();
                }
                else
                {
                    error.code = 0;
                }
                fbdo->_ss.jsonPtr->clear();
                fbdo->_ss.dataPtr->clear();
            }
            //JSON Array payload
            else if (fbdo->_ss.cfs.payload[0] == '[')
                error.code = 0;

            fbdo->_ss.content_length = response.payloadLen;
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

#endif //ENABLE