/**
 * Google's Cloud Firestore class, Forestore.cpp version 1.1.17
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

bool FB_Firestore::mExportDocuments(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr bucketID, MB_StringPtr storagePath, MB_StringPtr collectionIds)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_export_docs;
    req.projectId = projectId;
    req.databaseId = databaseId;

    MB_String outputUriPrefix;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    if (!fbdo->session.arrPtr)
        fbdo->session.arrPtr = new FirebaseJsonArray();

    fbdo->session.jsonPtr->clear();
    fbdo->session.arrPtr->clear();

    outputUriPrefix += fb_esp_pgm_str_350;
    outputUriPrefix += bucketID;
    outputUriPrefix += fb_esp_pgm_str_1;
    outputUriPrefix += storagePath;

    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_347), outputUriPrefix.c_str());

    MB_String _collectionIds = collectionIds;

    if (_collectionIds.length() > 0)
    {
        MB_VECTOR<MB_String> colIds;
        ut->splitTk(_collectionIds, colIds, ",");
        for (size_t i = 0; i < colIds.size(); i++)
            fbdo->session.arrPtr->add(colIds[i].c_str());

        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_346), *fbdo->session.arrPtr);
    }

    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();
    fbdo->session.arrPtr->clear();

    bool ret = sendRequest(fbdo, &req);
    req.payload.clear();
    return ret;
}

bool FB_Firestore::mImportDocuments(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr bucketID, MB_StringPtr storagePath, MB_StringPtr collectionIds)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_import_docs;
    req.projectId = projectId;
    req.databaseId = databaseId;

    MB_String inputUriPrefix;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    if (!fbdo->session.arrPtr)
        fbdo->session.arrPtr = new FirebaseJsonArray();

    fbdo->session.jsonPtr->clear();
    fbdo->session.arrPtr->clear();

    inputUriPrefix += fb_esp_pgm_str_350;
    inputUriPrefix += bucketID;
    inputUriPrefix += fb_esp_pgm_str_1;
    inputUriPrefix += storagePath;

    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_348), inputUriPrefix.c_str());

    MB_String _collectionIds = collectionIds;

    if (_collectionIds.length() > 0)
    {
        MB_VECTOR<MB_String> colIds;
        ut->splitTk(_collectionIds, colIds, ",");
        for (size_t i = 0; i < colIds.size(); i++)
            fbdo->session.arrPtr->add(colIds[i].c_str());

        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_346), *fbdo->session.arrPtr);
    }

    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();
    fbdo->session.arrPtr->clear();

    bool ret = sendRequest(fbdo, &req);
    req.payload.clear();
    return ret;
}

bool FB_Firestore::mCreateDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr documentPath, MB_StringPtr content, MB_StringPtr mask)
{
    size_t count = 0;
    MB_String collectionId, documentId;
    collectionId = documentPath;
    size_t p = collectionId.find_last_of("/");
    MB_String _documentPath = documentPath;
    for (size_t i = 0; i < _documentPath.length(); i++)
    {
        if (_documentPath[i] == '/')
            count++;
    }

    if (p != MB_String::npos && count % 2 > 0)
    {
        documentId = collectionId.substr(p + 1, collectionId.length() - p - 1);
        collectionId = collectionId.substr(0, p);
    }

    return createDocument(fbdo, projectId, databaseId, collectionId.c_str(), documentId.c_str(), content, mask);
}

bool FB_Firestore::mCreateDocument2(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr collectionId, MB_StringPtr documentId, MB_StringPtr content, MB_StringPtr mask)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_create_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentId = documentId;
    req.collectionId = collectionId;
    req.payload = content;
    req.mask = mask;
    if (Signer.getCfg())
        req.uploadCallback = Signer.getCfg()->cfs.upload_callback;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mPatchDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr documentPath, MB_StringPtr content, MB_StringPtr updateMask, MB_StringPtr mask, MB_StringPtr exists, MB_StringPtr updateTime)
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
    if (Signer.getCfg())
        req.uploadCallback = Signer.getCfg()->cfs.upload_callback;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::setFieldTransform(FirebaseJson *json, struct fb_esp_firestore_document_write_field_transforms_t *field_transforms)
{
    MB_String path;
    bool hasTransform = false;
    json->clear();

    if (field_transforms->transform_content.length() > 0)
    {
        if (field_transforms->fieldPath.length() > 0)
        {
            path = fb_esp_pgm_str_557;
            json->add(path.c_str(), field_transforms->fieldPath);
            hasTransform = true;
        }

        if (field_transforms->transform_type == fb_esp_firestore_transform_type_set_to_server_value)
        {
            path = fb_esp_pgm_str_558;
            json->add(path.c_str(), field_transforms->transform_content);
            hasTransform = true;
        }
        else if (field_transforms->transform_type != fb_esp_firestore_transform_type_undefined)
        {
            FirebaseJson js;

            if (field_transforms->transform_type == fb_esp_firestore_transform_type_increment)
                path = fb_esp_pgm_str_559;
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_maaximum)
                path = fb_esp_pgm_str_560;
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_minimum)
                path = fb_esp_pgm_str_561;
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_append_missing_elements)
                path = fb_esp_pgm_str_562;
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_remove_all_from_array)
                path = fb_esp_pgm_str_563;

            js.clear();
            js.setJsonData(field_transforms->transform_content);
            json->add(path.c_str(), js);
            js.clear();
            hasTransform = true;
        }
    }

    return hasTransform;
}

bool FB_Firestore::mCommitDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_VECTOR<struct fb_esp_firestore_document_write_t> writes, MB_StringPtr transaction, bool async)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_commit_document;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.transaction = transaction;
    req.async = async;
    if (Signer.getCfg())
        req.uploadCallback = Signer.getCfg()->cfs.upload_callback;

    if (writes.size() > 0)
    {
        MB_String path, updateMaskPath, docPathBase, docPath;
        updateMaskPath += fb_esp_pgm_str_400;
        updateMaskPath += fb_esp_pgm_str_1;
        updateMaskPath += fb_esp_pgm_str_556;

        FirebaseJson json;
        bool hasCurDoc = false;

        MB_String *writesArr = new MB_String[writes.size()];

        if (!fbdo->session.jsonPtr)
            fbdo->session.jsonPtr = new FirebaseJson();

        if (!fbdo->session.arrPtr)
            fbdo->session.arrPtr = new FirebaseJsonArray();

        fbdo->session.jsonPtr->clear();
        fbdo->session.arrPtr->clear();

        docPathBase = fb_esp_pgm_str_395;

        if (req.projectId.length() == 0)
            docPathBase += Signer.getCfg()->service_account.data.project_id;
        else
            docPathBase += req.projectId;

        docPathBase += fb_esp_pgm_str_341;
        if (req.databaseId.length() > 0)
            docPathBase += req.databaseId;
        else
            docPathBase += fb_esp_pgm_str_342;

        docPathBase += fb_esp_pgm_str_351;

        for (size_t i = 0; i < writes.size(); i++)
        {
            struct fb_esp_firestore_document_write_t *write = &writes[i];

            json.clear();

            hasCurDoc = false;

            if (write->update_masks.length() > 0)
            {
                MB_VECTOR<MB_String> masks;
                ut->splitTk(write->update_masks, masks, ",");
                for (size_t j = 0; j < masks.size(); j++)
                    fbdo->session.arrPtr->add(masks[j].c_str());

                fbdo->session.jsonPtr->set(updateMaskPath.c_str(), *fbdo->session.arrPtr);
                fbdo->session.arrPtr->clear();
            }

            if (setFieldTransform(&json, &write->update_transforms))
            {
                path = fb_esp_pgm_str_567;
                fbdo->session.jsonPtr->add(path.c_str(), json);
                json.clear();
            }

            if (write->current_document.exists.length() > 0 || write->current_document.update_time.length() > 0)
            {

                if (write->current_document.exists.length() > 0)
                {

                    if (strcmp(write->current_document.exists.c_str(), pgm2Str(fb_esp_pgm_str_107)) == 0)
                    {
                        json.add(pgm2Str(fb_esp_pgm_str_569), pgm2Str(fb_esp_pgm_str_107));
                        hasCurDoc = true;
                    }
                    else
                    {
                        if (strcmp(write->current_document.exists.c_str(), pgm2Str(fb_esp_pgm_str_106)) == 0)
                        {
                            json.add(pgm2Str(fb_esp_pgm_str_569), (const char *)pgm2Str(fb_esp_pgm_str_106));
                            hasCurDoc = true;
                        }
                    }
                }
                else if (write->current_document.update_time.length() > 0 && !hasCurDoc)
                {
                    json.add(pgm2Str(fb_esp_pgm_str_467), write->current_document.update_time);
                    hasCurDoc = true;
                }

                if (hasCurDoc)
                {
                    path = fb_esp_pgm_str_566;
                    fbdo->session.jsonPtr->add(path.c_str(), json);
                    json.clear();
                }
            }

            if (write->update_document_content.length() > 0 && write->update_document_path.length() > 0 && write->type == fb_esp_firestore_document_write_type_update)
            {
                docPath = docPathBase;

                if (write->update_document_path[0] != '/')
                    docPath += fb_esp_pgm_str_1;

                docPath += write->update_document_path;

                path = fb_esp_pgm_str_274;
                json.setJsonData(write->update_document_content);
                json.add(path.c_str(), docPath.c_str());

                path = fb_esp_pgm_str_118;
                fbdo->session.jsonPtr->add(path.c_str(), json);
                json.clear();
            }
            else if (write->delete_document_path.length() > 0 && write->type == fb_esp_firestore_document_write_type_delete)
            {
                docPath = docPathBase;

                if (write->delete_document_path[0] != '/')
                    docPath += fb_esp_pgm_str_1;

                docPath += write->delete_document_path;

                path = fb_esp_pgm_str_119;
                fbdo->session.jsonPtr->add(path.c_str(), docPath.c_str());
            }
            else if (write->document_transform.transform_document_path.length() > 0 && write->document_transform.field_transforms.size() > 0 && write->type == fb_esp_firestore_document_write_type_transform)
            {
                fbdo->session.arrPtr->clear();

                for (size_t j = 0; j < write->document_transform.field_transforms.size(); j++)
                {
                    json.clear();
                    if (setFieldTransform(&json, &write->document_transform.field_transforms[j]))
                        fbdo->session.arrPtr->add(json);
                }

                json.clear();

                docPath = docPathBase;

                if (write->document_transform.transform_document_path[0] != '/')
                    docPath += fb_esp_pgm_str_1;

                docPath += write->document_transform.transform_document_path;

                path = fb_esp_pgm_str_564;
                json.add(path.c_str(), docPath.c_str());

                path = fb_esp_pgm_str_565;
                json.add(path.c_str(), *fbdo->session.arrPtr);
                fbdo->session.arrPtr->clear();

                path = fb_esp_pgm_str_568;
                fbdo->session.jsonPtr->add(path.c_str(), json);
                json.clear();
            }

            writesArr[i] = fbdo->session.jsonPtr->raw();
            fbdo->session.jsonPtr->clear();
            fbdo->session.arrPtr->clear();
        }

        for (size_t i = 0; i < writes.size(); i++)
        {
            fbdo->session.jsonPtr->setJsonData(writesArr[i].c_str());
            fbdo->session.arrPtr->add(*fbdo->session.jsonPtr);
            fbdo->session.jsonPtr->clear();
            writesArr[i].clear();
        }

        delete[] writesArr;
        writesArr = nullptr;

        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_555), *fbdo->session.arrPtr);
        fbdo->session.arrPtr->clear();

        MB_String _transaction = transaction;

        if (_transaction.length() > 0)
        {
            fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_537), _transaction);
        }

        req.payload = fbdo->session.jsonPtr->raw();
        fbdo->session.jsonPtr->clear();
    }

    bool ret = sendRequest(fbdo, &req);
    req.payload.clear();
    return ret;
}

bool FB_Firestore::mGetDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr documentPath, MB_StringPtr mask, MB_StringPtr transaction, MB_StringPtr readTime)
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

bool FB_Firestore::mBeginTransaction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, TransactionOptions *transactionOptions)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_begin_transaction;
    req.projectId = projectId;
    req.databaseId = databaseId;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();

    if (transactionOptions)
    {
        if (transactionOptions->readOnly.readTime.length() > 0)
        {
            fbdo->session.jsonPtr->set(pgm2Str(fb_esp_pgm_str_571), transactionOptions->readOnly.readTime);
        }
        else if (transactionOptions->readWrite.retryTransaction.length() > 0)
        {
            fbdo->session.jsonPtr->set(pgm2Str(fb_esp_pgm_str_572), transactionOptions->readWrite.retryTransaction);
        }
    }

    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();
    bool ret = sendRequest(fbdo, &req);
    req.payload.clear();
    return ret;
}

bool FB_Firestore::mRollback(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr transaction)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_rollback;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.async = false;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();

    MB_String _transaction = transaction;

    if (_transaction.length() > 0)
    {
        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_537), _transaction);
    }
    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();
    bool ret = sendRequest(fbdo, &req);
    req.payload.clear();
    return ret;
}

bool FB_Firestore::mRunQuery(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr documentPath, FirebaseJson *structuredQuery, fb_esp_firestore_consistency_mode consistencyMode, MB_StringPtr consistency)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_run_query;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();
    if (consistencyMode != fb_esp_firestore_consistency_mode_undefined)
    {
        MB_String _consistency = consistency;
        if (consistencyMode != fb_esp_firestore_consistency_mode_transaction)
            fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_537), _consistency);
        else if (consistencyMode != fb_esp_firestore_consistency_mode_newTransaction)
            fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_538), _consistency);
        else if (consistencyMode != fb_esp_firestore_consistency_mode_readTime)
            fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_539), _consistency);
    }

    if (structuredQuery)
        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_536), *structuredQuery);
    else
    {
        static FirebaseJson js;
        fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_536), js);
    }

    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();

    bool ret = sendRequest(fbdo, &req);
    req.payload.clear();
    return ret;
}

bool FB_Firestore::mDeleteDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr documentPath, MB_StringPtr exists, MB_StringPtr updateTime)
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

bool FB_Firestore::mListDocuments(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr collectionId, MB_StringPtr pageSize, MB_StringPtr pageToken, MB_StringPtr orderBy, MB_StringPtr mask, bool showMissing)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_list_doc;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.collectionId = collectionId;
    MB_String _pageSize = pageSize;
    req.pageSize = atoi(_pageSize.c_str());
    req.pageToken = pageToken;
    req.orderBy = orderBy;
    req.mask = mask;
    req.showMissing = showMissing;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mListCollectionIds(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr documentPath, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct fb_esp_firestore_req_t req;
    req.requestType = fb_esp_firestore_request_type_list_collection;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentPath = documentPath;

    if (!fbdo->session.jsonPtr)
        fbdo->session.jsonPtr = new FirebaseJson();

    fbdo->session.jsonPtr->clear();
    MB_String _pageSize = pageSize;
    MB_String _pageToken = pageToken;
    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_357), atoi(_pageSize.c_str()));
    fbdo->session.jsonPtr->add(pgm2Str(fb_esp_pgm_str_358), _pageToken);

    req.payload = fbdo->session.jsonPtr->raw();
    fbdo->session.jsonPtr->clear();
    bool ret = sendRequest(fbdo, &req);
    req.payload.clear();
    return ret;
}

void FB_Firestore::begin(UtilsClass *u)
{
    ut = u;
}

bool FB_Firestore::sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
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

    Signer.getCfg()->internal.fb_processing = true;

    fbdo->session.cfs.payload.clear();

    // close session if async mode changes
    if (fbdo->session.cfs.async && !req->async)
        fbdo->session.last_conn_ms = 0;

    connect(fbdo);

    req->requestTime = millis();

    return firestore_sendRequest(fbdo, req);
}

bool FB_Firestore::firestore_sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
{
    bool ret = false;
    MB_String header;
    if (req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_list_doc)
        header += fb_esp_pgm_str_25;
    else if (req->requestType == fb_esp_firestore_request_type_rollback || req->requestType == fb_esp_firestore_request_type_begin_transaction || req->requestType == fb_esp_firestore_request_type_commit_document || req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_export_docs || req->requestType == fb_esp_firestore_request_type_import_docs || req->requestType == fb_esp_firestore_request_type_create_doc)
        header += fb_esp_pgm_str_24;
    else if (req->requestType == fb_esp_firestore_request_type_patch_doc)
        header += fb_esp_pgm_str_26;
    else if (req->requestType == fb_esp_firestore_request_type_delete_doc)
        header += fb_esp_pgm_str_27;

    header += fb_esp_pgm_str_6;
    header += fb_esp_pgm_str_326;

    if (req->projectId.length() == 0)
        header += Signer.getCfg()->service_account.data.project_id;
    else
        header += req->projectId;

    header += fb_esp_pgm_str_341;
    if (req->databaseId.length() > 0)
        header += req->databaseId;
    else
        header += fb_esp_pgm_str_342;

    if (req->requestType == fb_esp_firestore_request_type_export_docs)
        header += fb_esp_pgm_str_344;
    else if (req->requestType == fb_esp_firestore_request_type_import_docs)
        header += fb_esp_pgm_str_345;
    else if (req->requestType == fb_esp_firestore_request_type_begin_transaction)
    {
        header += fb_esp_pgm_str_351;
        header += fb_esp_pgm_str_573;
    }
    else if (req->requestType == fb_esp_firestore_request_type_rollback)
    {
        header += fb_esp_pgm_str_351;
        header += fb_esp_pgm_str_574;
    }
    else if (req->requestType == fb_esp_firestore_request_type_commit_document || req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_list_doc || req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_create_doc || req->requestType == fb_esp_firestore_request_type_patch_doc || req->requestType == fb_esp_firestore_request_type_delete_doc)
    {
        header += fb_esp_pgm_str_351;

        bool hasParam = false;
        if (req->requestType == fb_esp_firestore_request_type_create_doc)
        {
            if (req->collectionId.length() > 0)
            {
                if (req->collectionId[0] != '/')
                    header += fb_esp_pgm_str_1;
            }
            else
                header += fb_esp_pgm_str_1;

            header += req->collectionId;
            header += fb_esp_pgm_str_343;
            header += req->documentId;
            hasParam = true;
        }
        else if (req->requestType == fb_esp_firestore_request_type_run_query || req->requestType == fb_esp_firestore_request_type_list_collection || req->requestType == fb_esp_firestore_request_type_get_doc || req->requestType == fb_esp_firestore_request_type_patch_doc || req->requestType == fb_esp_firestore_request_type_delete_doc)
        {
            if (req->documentPath.length() > 0)
            {
                if (req->documentPath[0] != '/')
                    header += fb_esp_pgm_str_1;
            }
            else
                header += fb_esp_pgm_str_1;

            header += req->documentPath;
            if (req->requestType == fb_esp_firestore_request_type_list_collection)
                header += fb_esp_pgm_str_362;
            else if (req->requestType == fb_esp_firestore_request_type_run_query)
                header += fb_esp_pgm_str_535;
        }
        else if (req->requestType == fb_esp_firestore_request_type_list_doc)
        {
            if (req->collectionId.length() > 0)
            {
                if (req->collectionId[0] != '/')
                    header += fb_esp_pgm_str_1;
            }
            else
                header += fb_esp_pgm_str_1;

            header += req->collectionId;
            header += fb_esp_pgm_str_173;
            header += fb_esp_pgm_str_357;
            header += fb_esp_pgm_str_361;
            header += req->pageSize;
            header += fb_esp_pgm_str_172;
            header += fb_esp_pgm_str_358;
            header += fb_esp_pgm_str_361;
            header += req->pageToken;
            header += fb_esp_pgm_str_172;
            header += fb_esp_pgm_str_359;
            header += req->orderBy;
            header += fb_esp_pgm_str_172;
            header += fb_esp_pgm_str_360;
            header += req->showMissing;
            hasParam = true;
        }

        if (req->requestType == fb_esp_firestore_request_type_patch_doc)
        {
            if (req->updateMask.length() > 0)
            {
                MB_VECTOR<MB_String> docMasks;
                ut->splitTk(req->updateMask, docMasks, ",");
                for (size_t i = 0; i < docMasks.size(); i++)
                {
                    if (!hasParam)
                        header += fb_esp_pgm_str_173;
                    else
                        header += fb_esp_pgm_str_172;
                    header += fb_esp_pgm_str_352;
                    header += docMasks[i];
                    hasParam = true;
                }
            }
        }
        else if (req->requestType == fb_esp_firestore_request_type_commit_document)
        {
            header += fb_esp_pgm_str_554;
            fbdo->session.cfs.async = req->async;
        }

        if (req->mask.length() > 0)
        {
            MB_VECTOR<MB_String> docMasks;
            ut->splitTk(req->mask, docMasks, ",");
            for (size_t i = 0; i < docMasks.size(); i++)
            {
                if (!hasParam)
                    header += fb_esp_pgm_str_173;
                else
                    header += fb_esp_pgm_str_172;

                header += fb_esp_pgm_str_349;
                header += docMasks[i];
                hasParam = true;
            }
        }

        if (req->requestType == fb_esp_firestore_request_type_get_doc)
        {
            if (req->transaction.length() > 0)
            {
                if (!hasParam)
                    header += fb_esp_pgm_str_173;
                else
                    header += fb_esp_pgm_str_172;
                header += fb_esp_pgm_str_355;
                header += req->transaction;
                hasParam = true;
            }

            if (req->readTime.length() > 0)
            {
                if (!hasParam)
                    header += fb_esp_pgm_str_173;
                else
                    header += fb_esp_pgm_str_172;
                header += fb_esp_pgm_str_356;
                header += req->readTime;
            }
        }
        else if (req->requestType == fb_esp_firestore_request_type_patch_doc || req->requestType == fb_esp_firestore_request_type_delete_doc)
        {
            if (req->exists.length() > 0)
            {
                if (!hasParam)
                    header += fb_esp_pgm_str_173;
                else
                    header += fb_esp_pgm_str_172;
                header += fb_esp_pgm_str_353;
                header += req->exists;
                hasParam = true;
            }
            if (req->updateTime.length() > 0)
            {
                if (!hasParam)
                    header += fb_esp_pgm_str_173;
                else
                    header += fb_esp_pgm_str_172;
                header += fb_esp_pgm_str_354;
                header += req->updateTime;
            }
        }
    }

    header += fb_esp_pgm_str_30;

    if (req->payload.length() > 0)
    {
        header += fb_esp_pgm_str_8;
        header += fb_esp_pgm_str_129;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_12;
        header += req->payload.length();
        header += fb_esp_pgm_str_21;
    }

    header += fb_esp_pgm_str_31;
    header += fb_esp_pgm_str_340;
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
    header += fb_esp_pgm_str_36;

    ut->getCustomHeaders(header);

    header += fb_esp_pgm_str_21;

    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    fbdo->tcpClient.send(header.c_str());
    if (fbdo->session.response.code > 0 && req->payload.length() > 0)
    {
        if (req->uploadCallback)
        {
            req->size = req->payload.length();
            CFS_UploadStatusInfo in;
            in.status = fb_esp_cfs_upload_status_init;
            in.size = req->size;
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
            ret = tcpSend(fbdo, req->payload.c_str(), req);
            if (ret > 0)
            {
                CFS_UploadStatusInfo in;
                in.status = fb_esp_cfs_upload_status_complete;
                in.errorMsg = fbdo->errorReason().c_str();
                sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
            }
            else
            {
                CFS_UploadStatusInfo in;
                in.status = fb_esp_cfs_upload_status_error;
                in.errorMsg = fbdo->errorReason().c_str();
                sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
            }
        }
        else
            fbdo->tcpClient.send(req->payload.c_str());
    }

    header.clear();
    req->payload.clear();

    if (fbdo->session.response.code > 0)
    {
        fbdo->session.connected = true;
        if (fbdo->session.cfs.async || handleResponse(fbdo, req))
        {
            Signer.getCfg()->internal.fb_processing = false;
            return true;
        }
    }
    else
        fbdo->session.connected = false;

    Signer.getCfg()->internal.fb_processing = false;

    return false;
}

void FB_Firestore::rescon(FirebaseData *fbdo, const char *host)
{
    if (fbdo->session.cert_updated || !fbdo->session.connected || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout || fbdo->session.con_mode != fb_esp_con_mode_firestore || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->session.host = host;
    fbdo->session.con_mode = fb_esp_con_mode_firestore;
}

bool FB_Firestore::connect(FirebaseData *fbdo)
{
    MB_String host = fb_esp_pgm_str_340;
    host += fb_esp_pgm_str_120;
    rescon(fbdo, host.c_str());
    fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
    return true;
}

bool FB_Firestore::handleResponse(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
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
    fbdo->session.cfs.payload.clear();

    fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->session.content_length = -1;
    fbdo->session.payload_length = 0;
    fbdo->session.chunked_encoding = false;
    fbdo->session.buffer_ovf = false;

    defaultChunkSize = 8192;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = fbdo->tcpClient.available();
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

            // Waring when large payload response may require the time for complete reading
            if (req->uploadCallback && millis() - req->requestTime > 10000 && fbdo->session.cfs.cbUploadInfo.status != fb_esp_cfs_upload_status_process_response)
            {
                req->requestTime = millis();
                fbdo->session.cfs.cbUploadInfo.status = fb_esp_cfs_upload_status_process_response;
                CFS_UploadStatusInfo in;
                in.status = fb_esp_cfs_upload_status_process_response;
                in.progress = 100;
                sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
            }

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

                            if (response.httpCode == 401)
                                Signer.authenticated = false;
                            else if (response.httpCode < 300)
                                Signer.authenticated = true;

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
                                if (payloadRead < payloadLen)
                                    fbdo->session.cfs.payload += pChunk;
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
        if (fbdo->session.cfs.payload.length() > 0)
        {
            if (fbdo->session.cfs.payload[0] == '{')
            {
                if (!fbdo->session.jsonPtr)
                    fbdo->session.jsonPtr = new FirebaseJson();
                if (!fbdo->session.dataPtr)
                    fbdo->session.dataPtr = new FirebaseJsonData();

                fbdo->session.jsonPtr->setJsonData(fbdo->session.cfs.payload.c_str());
                fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_257));

                if (fbdo->session.dataPtr->success)
                {
                    error.code = fbdo->session.dataPtr->to<int>();
                    fbdo->session.jsonPtr->get(*fbdo->session.dataPtr, pgm2Str(fb_esp_pgm_str_258));
                    if (fbdo->session.dataPtr->success)
                        fbdo->session.error = fbdo->session.dataPtr->to<const char *>();
                    fbdo->session.cfs.payload.clear();
                }
                else
                {
                    error.code = 0;
                }
                fbdo->session.jsonPtr->clear();
                fbdo->session.dataPtr->clear();
            }
            // JSON Array payload
            else if (fbdo->session.cfs.payload[0] == '[')
                error.code = 0;

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

void FB_Firestore::reportUploadProgress(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req, size_t readBytes)
{

    if (req->size == 0)
        return;

    int p = (float)readBytes / req->size * 100;

    if (req->progress != p && (p == 0 || p == 100 || req->progress + ESP_REPORT_PROGRESS_INTERVAL <= p))
    {
        req->progress = p;

        fbdo->tcpClient.dataTime = millis() - fbdo->tcpClient.dataStart;

        fbdo->session.cfs.cbUploadInfo.status = fb_esp_cfs_upload_status_upload;
        CFS_UploadStatusInfo in;
        in.status = fb_esp_cfs_upload_status_upload;
        in.progress = p;
        in.elapsedTime = fbdo->tcpClient.dataTime;
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
    }
}

void FB_Firestore::sendUploadCallback(FirebaseData *fbdo, CFS_UploadStatusInfo &in, CFS_UploadProgressCallback cb, CFS_UploadStatusInfo *out)
{

    fbdo->session.cfs.cbUploadInfo.status = in.status;
    fbdo->session.cfs.cbUploadInfo.errorMsg = in.errorMsg;
    fbdo->session.cfs.cbUploadInfo.progress = in.progress;
    fbdo->session.cfs.cbUploadInfo.size = in.size;
    fbdo->session.cfs.cbUploadInfo.elapsedTime = in.elapsedTime;

    if (cb)
        cb(fbdo->session.cfs.cbUploadInfo);

    if (out)
    {
        out->errorMsg = fbdo->session.cfs.cbUploadInfo.errorMsg;
        out->status = fbdo->session.cfs.cbUploadInfo.status;
        out->progress = fbdo->session.cfs.cbUploadInfo.progress;
    }
}

int FB_Firestore::tcpSend(FirebaseData *fbdo, const char *data, struct fb_esp_firestore_req_t *req)
{
    size_t len = strlen(data);

#if defined(ESP8266)
    if (fbdo->session.bssl_tx_size < 512)
        fbdo->session.bssl_tx_size = 512;
    int chunkSize = fbdo->session.bssl_tx_size;
#else
    int chunkSize = 4096;
#endif

    int sent = 0;
    int ret = 0;

    while (sent < (int)len)
    {
        if (sent + chunkSize > (int)len)
            chunkSize = len - sent;

        reportUploadProgress(fbdo, req, sent);

        ret = fbdo->tcpClient.send(data + sent, chunkSize);

        if (ret < 0)
            return ret;

        sent += chunkSize;
    }

    reportUploadProgress(fbdo, req, sent);

    return ret;
}

#endif

#endif // ENABLE