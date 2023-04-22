#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Cloud Firestore class, Forestore.cpp version 1.2.5
 *
 * This library supports Espressif ESP8266, ESP32 and RP2040 Pico
 *
 * Created April 5, 2023
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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
#include "mbfs/MB_MCU.h"
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

void FB_Firestore::makeRequest(struct fb_esp_firestore_req_t &req, fb_esp_firestore_request_type type,
                               MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr documentId,
                               MB_StringPtr collectionId)
{
    req.requestType = type;
    req.projectId = projectId;
    req.databaseId = databaseId;
    req.documentId = documentId;
    req.collectionId = collectionId;
}

bool FB_Firestore::mImportExportDocuments(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                          MB_StringPtr bucketID, MB_StringPtr storagePath, MB_StringPtr collectionIds,
                                          bool isImport)
{
    struct fb_esp_firestore_req_t req;

    makeRequest(req, isImport ? fb_esp_firestore_request_type_import_docs : fb_esp_firestore_request_type_export_docs,
                projectId, databaseId, toStringPtr(""), toStringPtr(""));

    MB_String uriPrefix;

    fbdo->initJson();

    URLHelper::addGStorageURL(uriPrefix, MB_String(bucketID), MB_String(storagePath));
    JsonHelper::addString(fbdo->session.jsonPtr, isImport ? fb_esp_cfs_pgm_str_4 /* "inputUriPrefix" */
                                                          : fb_esp_cfs_pgm_str_3 /* "outputUriPrefix" */,
                          uriPrefix);
    JsonHelper::addTokens(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_2 /* "collectionIds" */, MB_String(collectionIds));

    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    fbdo->clearJson();
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mCreateDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                   MB_StringPtr documentPath, MB_StringPtr content, MB_StringPtr mask)
{
    size_t count = 0;
    MB_String collectionId, documentId;
    collectionId = documentPath;
    size_t p = collectionId.find_last_of("/");
    MB_String _documentPath = documentPath;

    for (size_t i = 0; i < _documentPath.length(); i++)
        count += _documentPath[i] == '/' ? 1 : 0;

    if (p != MB_String::npos && count % 2 > 0)
    {
        documentId = collectionId.substr(p + 1, collectionId.length() - p - 1);
        collectionId = collectionId.substr(0, p);
    }

    return createDocument(fbdo, projectId, databaseId, collectionId.c_str(), documentId.c_str(), content, mask);
}

bool FB_Firestore::mCreateDocument2(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                    MB_StringPtr collectionId, MB_StringPtr documentId, MB_StringPtr content,
                                    MB_StringPtr mask)
{
    struct fb_esp_firestore_req_t req;

    makeRequest(req, fb_esp_firestore_request_type_create_doc, projectId, databaseId, documentId, collectionId);
    req.payload = content;
    req.mask = mask;
    if (Signer.config)
        req.uploadCallback = Signer.config->cfs.upload_callback;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mPatchDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                  MB_StringPtr documentPath, MB_StringPtr content, MB_StringPtr updateMask,
                                  MB_StringPtr mask, MB_StringPtr exists, MB_StringPtr updateTime)
{
    struct fb_esp_firestore_req_t req;

    makeRequest(req, fb_esp_firestore_request_type_patch_doc, projectId, databaseId, toStringPtr(""), toStringPtr(""));

    req.documentPath = documentPath;
    req.payload = content;
    req.updateMask = updateMask;
    req.mask = mask;
    req.exists = exists;
    req.updateTime = updateTime;

    if (Signer.config)
        req.uploadCallback = Signer.config->cfs.upload_callback;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::setFieldTransform(FirebaseJson *json,
                                     struct fb_esp_firestore_document_write_field_transforms_t *field_transforms)
{
    bool hasTransform = false;
    json->clear();

    if (field_transforms->transform_content.length() > 0)
    {
        JsonHelper::addString(json, fb_esp_cfs_pgm_str_5 /* "fieldPath" */, field_transforms->fieldPath, hasTransform);

        if (field_transforms->transform_type == fb_esp_firestore_transform_type_set_to_server_value)
            JsonHelper::addString(json, fb_esp_cfs_pgm_str_6 /* "setToServerValue" */,
                                  field_transforms->transform_content, hasTransform);
        else if (field_transforms->transform_type != fb_esp_firestore_transform_type_undefined)
        {
            MB_String key;

            if (field_transforms->transform_type == fb_esp_firestore_transform_type_increment)
                key = fb_esp_cfs_pgm_str_7; // "increment"
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_maaximum)
                key = fb_esp_cfs_pgm_str_8; // "maximum"
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_minimum)
                key = fb_esp_cfs_pgm_str_9; // "minimum"
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_append_missing_elements)
                key = fb_esp_cfs_pgm_str_10; // "appendMissingElements"
            else if (field_transforms->transform_type == fb_esp_firestore_transform_type_remove_all_from_array)
                key = fb_esp_cfs_pgm_str_11; // "removeAllFromArray"

            FirebaseJson js(field_transforms->transform_content);
            json->add(key, js);
            hasTransform = true;
        }
    }

    return hasTransform;
}

bool FB_Firestore::mCommitDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                   MB_VECTOR<struct fb_esp_firestore_document_write_t> writes, MB_StringPtr transaction,
                                   bool async)
{
    struct fb_esp_firestore_req_t req;

    makeRequest(req, fb_esp_firestore_request_type_commit_document, projectId, databaseId, toStringPtr(""), toStringPtr(""));
    req.transaction = transaction;
    req.async = async;

    if (Signer.config)
        req.uploadCallback = Signer.config->cfs.upload_callback;

    parseWrites(fbdo, writes, req);

    if (writes.size() > 0)
    {
        JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_28 /* "transaction" */, MB_String(transaction));
        JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
        fbdo->clearJson();
    }

    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mBatchWrite(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                               MB_VECTOR<struct fb_esp_firestore_document_write_t> writes, FirebaseJson *labels)
{
    struct fb_esp_firestore_req_t req;

    makeRequest(req, fb_esp_firestore_request_type_batch_write_doc, projectId, databaseId, toStringPtr(""), toStringPtr(""));

    parseWrites(fbdo, writes, req);

    if (writes.size() > 0)
    {
        if (labels)
            JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_pgm_str_64 /* "labels" */, labels, false);
        JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
        fbdo->clearJson();
    }

    return sendRequest(fbdo, &req);
}

void FB_Firestore::parseWrites(FirebaseData *fbdo, MB_VECTOR<struct fb_esp_firestore_document_write_t> writes, struct fb_esp_firestore_req_t &req)
{
    if (writes.size() > 0)
    {
        MB_String path, updateMaskPath, docPathBase, docPath;
        updateMaskPath += fb_esp_pgm_str_70; // "updateMask"
        updateMaskPath += fb_esp_pgm_str_1;   // "/"
        updateMaskPath += fb_esp_cfs_pgm_str_12; // "fieldPaths"

        FirebaseJson json;
        bool hasCurDoc = false;

        MB_String *writesArr = new MB_String[writes.size()];

        fbdo->initJson();

        docPathBase = Utils::makeDocPath(req, Signer.config->service_account.data.project_id);

        for (size_t i = 0; i < writes.size(); i++)
        {
            struct fb_esp_firestore_document_write_t *write = &writes[i];

            hasCurDoc = false;

            JsonHelper::addTokens(fbdo->session.jsonPtr, updateMaskPath.c_str(), write->update_masks);

            if (setFieldTransform(&json, &write->update_transforms))
                JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_13 /* "updateTransforms" */, &json, true);

            JsonHelper::addString(&json, fb_esp_cfs_pgm_str_14 /* "exists" */, write->current_document.exists, hasCurDoc);

            if (!hasCurDoc)
                JsonHelper::addString(&json, fb_esp_cfs_pgm_str_15 /* "updateTime" */,
                                      write->current_document.update_time, hasCurDoc);

            if (hasCurDoc)
                JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_16 /* "currentDocument" */, &json, true);

            if (write->update_document_content.length() > 0 &&
                write->update_document_path.length() > 0 &&
                write->type == fb_esp_firestore_document_write_type_update)
            {
                docPath = docPathBase;
                URLHelper::addPath(docPath, write->update_document_path);
                JsonHelper::setData(&json, write->update_document_content, false);
                JsonHelper::addString(&json, fb_esp_pgm_str_66 /* "name"*/, docPath);
                JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_pgm_str_68 /* "update"*/, &json, true);
            }
            else if (write->delete_document_path.length() > 0 && write->type == fb_esp_firestore_document_write_type_delete)
            {
                docPath = docPathBase;
                URLHelper::addPath(docPath, write->delete_document_path);
                JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_pgm_str_69 /* "delete"*/, docPath);
            }
            else if (write->document_transform.transform_document_path.length() > 0 &&
                     write->document_transform.field_transforms.size() > 0 &&
                     write->type == fb_esp_firestore_document_write_type_transform)
            {
                JsonHelper::arrayClear(fbdo->session.arrPtr);
                json.clear();

                for (size_t j = 0; j < write->document_transform.field_transforms.size(); j++)
                {
                    if (setFieldTransform(&json, &write->document_transform.field_transforms[j]))
                        JsonHelper::arrayAddObject(fbdo->session.arrPtr, &json, true);
                }

                docPath = docPathBase;
                URLHelper::addPath(docPath, write->document_transform.transform_document_path);
                JsonHelper::addString(&json, fb_esp_cfs_pgm_str_17 /* "document"*/, docPath);
                JsonHelper::addArray(&json, fb_esp_cfs_pgm_str_18 /* "fieldTransforms"*/, fbdo->session.arrPtr, true);
                JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_19 /* "transform"*/, &json, true);
            }

            writesArr[i].clear();
            JsonHelper::toString(fbdo->session.jsonPtr, writesArr[i], true);
            fbdo->clearJson();
        }

        for (size_t i = 0; i < writes.size(); i++)
            JsonHelper::arrayAddObjectString(fbdo->session.arrPtr, writesArr[i], true);

        delete[] writesArr;
        writesArr = nullptr;

        JsonHelper::addArray(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_20 /* "writes" */, fbdo->session.arrPtr, true);
    }
}

bool FB_Firestore::mGetDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                MB_StringPtr documentPath, MB_StringPtr mask, MB_StringPtr transaction,
                                FirebaseJson *newTransaction, MB_StringPtr readTime,
                                FirebaseData::FirestoreBatchOperationsCallback batchOperationCallback)
{
    struct fb_esp_firestore_req_t req;

    makeRequest(req, batchOperationCallback ? fb_esp_firestore_request_type_batch_get_doc : fb_esp_firestore_request_type_get_doc,
                projectId, databaseId, toStringPtr(""), toStringPtr(""));

    req.documentPath = documentPath;
    req.mask = mask;
    req.transaction = transaction;
    req.readTime = readTime;
    if (batchOperationCallback)
    {
        req.responseCallback = (FB_ResponseCallback)batchOperationCallback;

        MB_String docPathBase = Utils::makeDocPath(req, Signer.config->service_account.data.project_id);
        docPathBase += fb_esp_pgm_str_1; /* "/" */

        fbdo->initJson();

        JsonHelper::addTokens(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_21 /* "/documents" */, req.documentPath, docPathBase.c_str());
        if (req.mask.length() > 0)
        {
            MB_String path = fb_esp_cfs_pgm_str_22;
            path += fb_esp_pgm_str_1;
            path += fb_esp_cfs_pgm_str_12;
            JsonHelper::addTokens(fbdo->session.jsonPtr, path.c_str() /* "mask/fieldPaths" */, req.mask);
        }

        if (newTransaction)
            JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_23 /* newTransaction */, newTransaction, false);
        else if (req.transaction.length() > 0)
            JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_28 /* "transaction" */, req.transaction);
        else if (req.readTime.length() > 0)
            JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_24 /* "readTime" */, req.readTime);
        JsonHelper::toString(fbdo->session.jsonPtr, req.payload, false);
    }

    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mBeginTransaction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                     TransactionOptions *transactionOptions)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_begin_transaction, projectId, databaseId, toStringPtr(""), toStringPtr(""));
    fbdo->initJson();
    if (transactionOptions && !(transactionOptions->readOnly.readTime.length() > 0 &&
                                transactionOptions->readWrite.retryTransaction.length() > 0))
    {
        JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_25 /* "options/readOnly/readTime" */,
                              transactionOptions->readOnly.readTime);
        JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_26 /* "options/readWrite/retryTransaction" */,
                              transactionOptions->readWrite.retryTransaction);
    }

    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mRollback(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr transaction)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_rollback, projectId, databaseId, toStringPtr(""), toStringPtr(""));
    req.async = false;
    fbdo->initJson();
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_28 /* "transaction" */, MB_String(transaction));
    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mRunQuery(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                             MB_StringPtr documentPath, FirebaseJson *structuredQuery,
                             fb_esp_firestore_consistency_mode consistencyMode, MB_StringPtr consistency)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_run_query, projectId, databaseId, toStringPtr(""), toStringPtr(""));
    req.documentPath = documentPath;
    fbdo->initJson();
    if (consistencyMode != fb_esp_firestore_consistency_mode_undefined)
    {
        if (consistencyMode != fb_esp_firestore_consistency_mode_transaction)
            JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_28 /* "transaction" */, MB_String(consistency));
        else if (consistencyMode != fb_esp_firestore_consistency_mode_newTransaction)
            JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_29 /* "newTransaction" */, MB_String(consistency));
        else if (consistencyMode != fb_esp_firestore_consistency_mode_readTime)
            JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_24 /* "readTime" */, MB_String(consistency));
    }
    JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_27 /* "structuredQuery" */, structuredQuery, false);
    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mDeleteDocument(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                   MB_StringPtr documentPath, MB_StringPtr exists, MB_StringPtr updateTime)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_delete_doc, projectId, databaseId, toStringPtr(""), toStringPtr(""));
    req.documentPath = documentPath;
    req.exists = exists;
    req.updateTime = updateTime;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mListDocuments(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                  MB_StringPtr collectionId, MB_StringPtr pageSize, MB_StringPtr pageToken,
                                  MB_StringPtr orderBy, MB_StringPtr mask, bool showMissing)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_list_doc, projectId, databaseId, toStringPtr(""), collectionId);
    req.pageSize = atoi(stringPtr2Str(pageSize));
    req.pageToken = pageToken;
    req.orderBy = orderBy;
    req.mask = mask;
    req.showMissing = showMissing;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mListCollectionIds(FirebaseData *fbdo, MB_StringPtr projectId,
                                      MB_StringPtr databaseId, MB_StringPtr documentPath, MB_StringPtr pageSize,
                                      MB_StringPtr pageToken)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_list_collection, projectId, databaseId, toStringPtr(""), toStringPtr(""));
    req.documentPath = documentPath;
    fbdo->initJson();
    JsonHelper::addNumberString(fbdo->session.jsonPtr, fb_esp_pgm_str_63 /* "pageSize" */, MB_String(pageSize));
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_pgm_str_65 /* pageToken" */, stringPtr2Str(pageToken));
    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mCreateIndex(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                MB_StringPtr collectionId, MB_StringPtr apiScope, MB_StringPtr queryScope,
                                FirebaseJsonArray *fields)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_create_index, projectId, databaseId, toStringPtr(""), collectionId);
    fbdo->initJson();
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_1 /* queryScope" */, stringPtr2Str(queryScope));
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_30 /* apiScope" */, stringPtr2Str(apiScope));
    JsonHelper::addArray(fbdo->session.jsonPtr, fb_esp_cfs_pgm_str_31 /* fields" */, fields, false);

    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mGetDeleteIndex(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId,
                                   MB_StringPtr collectionId, MB_StringPtr indexId, int type)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, type == 0 ? fb_esp_firestore_request_type_get_index : fb_esp_firestore_request_type_delete_index,
                projectId, databaseId, toStringPtr(""), collectionId);
    req.payload = indexId;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::mListIndex(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr databaseId, MB_StringPtr collectionId,
                              MB_StringPtr filter, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct fb_esp_firestore_req_t req;
    makeRequest(req, fb_esp_firestore_request_type_list_index, projectId, databaseId, toStringPtr(""), collectionId);
    req.pageSize = atoi(stringPtr2Str(pageSize));
    req.pageToken = pageToken;
    req.payload = filter;
    return sendRequest(fbdo, &req);
}

bool FB_Firestore::sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
{
    if (fbdo->tcpClient.reserved)
        return false;
    fbdo->session.http_code = 0;
    if (!Signer.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

#ifdef ENABLE_RTDB
    if (fbdo->session.rtdb.pause)
        return true;
#endif

    if (!fbdo->reconnect() || !Signer.tokenReady())
        return false;

    if (fbdo->session.long_running_task > 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Signer.config->internal.fb_processing)
        return false;

    Signer.config->internal.fb_processing = true;

    fbdo->session.cfs.payload.clear();

    // close session if async mode changes
    if (fbdo->session.cfs.async && !req->async)
        fbdo->session.last_conn_ms = 0;

    connect(fbdo);
    req->requestTime = millis();
    
    bool ret = firestore_sendRequest(fbdo, req);
    if (!ret)
        fbdo->closeSession();

    return ret;
}

bool FB_Firestore::firestore_sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
{
    fbdo->_responseCallback = req->responseCallback;
    bool ret = false;
    bool hasParam = false;
    MB_String header;
    fb_esp_request_method method = http_undefined;
    if (req->requestType == fb_esp_firestore_request_type_get_doc ||
        req->requestType == fb_esp_firestore_request_type_list_doc ||
        req->requestType == fb_esp_firestore_request_type_list_index ||
        req->requestType == fb_esp_firestore_request_type_get_index)
        method = http_get;
    else if (req->requestType == fb_esp_firestore_request_type_rollback ||
             req->requestType == fb_esp_firestore_request_type_begin_transaction ||
             req->requestType == fb_esp_firestore_request_type_commit_document ||
             req->requestType == fb_esp_firestore_request_type_batch_write_doc ||
             req->requestType == fb_esp_firestore_request_type_run_query ||
             req->requestType == fb_esp_firestore_request_type_list_collection ||
             req->requestType == fb_esp_firestore_request_type_export_docs ||
             req->requestType == fb_esp_firestore_request_type_import_docs ||
             req->requestType == fb_esp_firestore_request_type_create_doc ||
             req->requestType == fb_esp_firestore_request_type_batch_get_doc ||
             req->requestType == fb_esp_firestore_request_type_create_index)
        method = http_post;
    else if (req->requestType == fb_esp_firestore_request_type_patch_doc)
        method = http_patch;
    else if (req->requestType == fb_esp_firestore_request_type_delete_doc ||
             req->requestType == fb_esp_firestore_request_type_delete_index)
        method = http_delete;

    if (method != http_undefined)
        HttpHelper::addRequestHeaderFirst(header, method);

    URLHelper::addGAPIv1Path(header);

    header += req->projectId.length() == 0 ? Signer.config->service_account.data.project_id : req->projectId;
    header += fb_esp_cfs_pgm_str_32; // "/databases/"
    header += req->databaseId.length() > 0 ? req->databaseId : fb_esp_cfs_pgm_str_33 /* "(default)" */;
    if (req->requestType == fb_esp_firestore_request_type_export_docs)
        header += fb_esp_cfs_pgm_str_34; // ":exportDocuments"
    else if (req->requestType == fb_esp_firestore_request_type_import_docs)
        header += fb_esp_cfs_pgm_str_35; // ":importDocuments"
    else if (req->requestType == fb_esp_firestore_request_type_begin_transaction)
    {
        header += fb_esp_cfs_pgm_str_21; // "/documents"
        header += fb_esp_cfs_pgm_str_36; // ":beginTransaction"
    }
    else if (req->requestType == fb_esp_firestore_request_type_rollback)
    {
        header += fb_esp_cfs_pgm_str_21; // "/documents"
        header += fb_esp_cfs_pgm_str_37; // ":rollback"
    }
    else if (req->requestType == fb_esp_firestore_request_type_batch_get_doc)
    {
        header += fb_esp_cfs_pgm_str_21; // "/documents"
        header += fb_esp_cfs_pgm_str_38; // ":batchGet"
    }
    else if (req->requestType == fb_esp_firestore_request_type_batch_write_doc)
    {
        header += fb_esp_cfs_pgm_str_21; // "/documents"
        header += fb_esp_cfs_pgm_str_39; // ":batchWrite"
    }
    else if (req->requestType == fb_esp_firestore_request_type_commit_document ||
             req->requestType == fb_esp_firestore_request_type_run_query ||
             req->requestType == fb_esp_firestore_request_type_list_collection ||
             req->requestType == fb_esp_firestore_request_type_list_doc ||
             req->requestType == fb_esp_firestore_request_type_get_doc ||
             req->requestType == fb_esp_firestore_request_type_create_doc ||
             req->requestType == fb_esp_firestore_request_type_patch_doc ||
             req->requestType == fb_esp_firestore_request_type_delete_doc)
    {
        header += fb_esp_cfs_pgm_str_21; // "/documents"

        if (req->requestType == fb_esp_firestore_request_type_create_doc)
        {
            URLHelper::addPath(header, req->collectionId);
            URLHelper::addParam(header, fb_esp_cfs_pgm_str_40 /* "documentId=" */, req->documentId, hasParam);
        }
        else if (req->requestType == fb_esp_firestore_request_type_run_query ||
                 req->requestType == fb_esp_firestore_request_type_list_collection ||
                 req->requestType == fb_esp_firestore_request_type_get_doc ||
                 req->requestType == fb_esp_firestore_request_type_patch_doc ||
                 req->requestType == fb_esp_firestore_request_type_delete_doc)
        {
            URLHelper::addPath(header, req->documentPath);
            header += (req->requestType == fb_esp_firestore_request_type_list_collection)
                          ? fb_esp_cfs_pgm_str_41 /* ":listCollectionIds" */
                      : req->requestType == fb_esp_firestore_request_type_run_query
                          ? fb_esp_cfs_pgm_str_42 /* ":runQuery" */
                          : "";
        }
        else if (req->requestType == fb_esp_firestore_request_type_list_doc)
        {
            URLHelper::addPath(header, req->collectionId);
            URLHelper::addParam(header, fb_esp_pgm_str_63 /* "pageSize" */, MB_String(req->pageSize), hasParam);
            URLHelper::addParam(header, fb_esp_pgm_str_65 /* "pageToken" */, req->pageToken, hasParam);
            URLHelper::addParam(header, fb_esp_cfs_pgm_str_43 /* "orderBy=" */, req->orderBy, hasParam);
            URLHelper::addParam(header, fb_esp_cfs_pgm_str_44 /* "showMissing=" */, MB_String(req->showMissing), hasParam);
        }

        if (req->requestType == fb_esp_firestore_request_type_patch_doc)
        {
            URLHelper::addParamsTokens(header, fb_esp_cfs_pgm_str_45 /* "updateMask.fieldPaths=" */, req->updateMask, hasParam);
        }
        else if (req->requestType == fb_esp_firestore_request_type_commit_document)
        {
            header += fb_esp_cfs_pgm_str_48; // ":commit"
            fbdo->session.cfs.async = req->async;
        }

        URLHelper::addParamsTokens(header, fb_esp_cfs_pgm_str_49 /* "mask.fieldPaths=" */, req->mask, hasParam);

        if (req->requestType == fb_esp_firestore_request_type_get_doc)
        {
            URLHelper::addParam(header, fb_esp_cfs_pgm_str_50 /* "transaction=" */, req->transaction, hasParam);
            URLHelper::addParam(header, fb_esp_cfs_pgm_str_51 /* "readTime=" */, req->readTime, hasParam);
        }
        else if (req->requestType == fb_esp_firestore_request_type_patch_doc ||
                 req->requestType == fb_esp_firestore_request_type_delete_doc)
        {
            URLHelper::addParam(header, fb_esp_cfs_pgm_str_46 /* "currentDocument.exists=" */, req->exists, hasParam);
            URLHelper::addParam(header, fb_esp_cfs_pgm_str_47 /* "currentDocument.updateTime=" */, req->updateTime, hasParam);
        }
    }
    else if (req->requestType == fb_esp_firestore_request_type_create_index ||
             req->requestType == fb_esp_firestore_request_type_delete_index ||
             req->requestType == fb_esp_firestore_request_type_get_index ||
             req->requestType == fb_esp_firestore_request_type_list_index)
    {
        header += fb_esp_cfs_pgm_str_52; // "/collectionGroups/"
        header += req->collectionId;
        header += fb_esp_cfs_pgm_str_53; // "/indexes"

        if (req->requestType == fb_esp_firestore_request_type_delete_index ||
            req->requestType == fb_esp_firestore_request_type_get_index)
        {
            header += fb_esp_pgm_str_1; /* "/" */
            header += req->payload;
        }
        else if (req->requestType == fb_esp_firestore_request_type_list_index)
        {
            if (req->pageSize > -1)
                URLHelper::addParam(header, fb_esp_pgm_str_63 /* "pageSize" */, MB_String(req->pageSize), hasParam);
            URLHelper::addParam(header, fb_esp_pgm_str_65 /* "pageToken" */, req->pageToken, hasParam);
            if (req->payload.length() > 0)
                URLHelper::addParam(header, fb_esp_cfs_pgm_str_54 /* "filter" */, req->payload, hasParam);
        }
    }

    HttpHelper::addRequestHeaderLast(header);

    if (req->payload.length() > 0 && (method == http_post || method == http_patch))
    {
        HttpHelper::addContentTypeHeader(header, fb_esp_pgm_str_62 /* "application/json" */);
        HttpHelper::addContentLengthHeader(header, req->payload.length());
    }

    HttpHelper::addGAPIsHostHeader(header, fb_esp_cfs_pgm_str_55 /* "firestore." */);

    if (!Signer.config->signer.test_mode)
    {

        HttpHelper::addAuthHeaderFirst(header, Signer.getTokenType());
        fbdo->tcpClient.send(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        fbdo->tcpClient.send(Signer.getToken());

        if (fbdo->session.response.code < 0)
            return false;

        HttpHelper::addNewLine(header);
    }

    HttpHelper::addUAHeader(header);
    bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE)
    keepAlive = true;
#endif
    HttpHelper::addConnectionHeader(header, keepAlive);
    HttpHelper::getCustomHeaders(header, Signer.config->signer.customHeaders);
    HttpHelper::addNewLine(header);
    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;
    fbdo->tcpClient.send(header.c_str());

    if (fbdo->session.response.code < 0)
        return false;

    if (fbdo->session.response.code > 0 && req->payload.length() > 0 &&
        (method == http_post || method == http_patch))
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

    if (fbdo->session.response.code > 0)
    {
        fbdo->session.connected = true;
        if (fbdo->session.cfs.async || handleResponse(fbdo, req))
        {
            Signer.config->internal.fb_processing = false;
            return true;
        }
    }
    else
        fbdo->session.connected = false;

    Signer.config->internal.fb_processing = false;
    return false;
}

void FB_Firestore::rescon(FirebaseData *fbdo, const char *host)
{
    fbdo->_responseCallback = NULL;

    if (fbdo->session.cert_updated || !fbdo->session.connected ||
        millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout ||
        fbdo->session.con_mode != fb_esp_con_mode_firestore ||
        strcmp(host, fbdo->session.host.c_str()) != 0)
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
    MB_String host;
    HttpHelper::addGAPIsHost(host, fb_esp_cfs_pgm_str_55 /* "firestore." */);
    rescon(fbdo, host.c_str());
    fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
    return true;
}

bool FB_Firestore::handleResponse(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req)
{
    if (!fbdo->reconnect())
        return false;

    struct server_response_data_t response;
    struct fb_esp_tcp_response_handler_t tcpHandler;

    HttpHelper::initTCPSession(fbdo->session);
    HttpHelper::intTCPHandler(fbdo->tcpClient.client, tcpHandler, 2048, fbdo->session.resp_size, nullptr, false);

    if (!fbdo->waitResponse(tcpHandler))
        return false;

    bool complete = false;

    while (tcpHandler.available() > 0 /* data available to read payload */ ||
           tcpHandler.payloadRead < response.contentLen /* incomplete content read  */ || !complete)
    {
        tcpHandler.dataTime = millis();

        // Waring when large payload response may require the time for complete reading
        if (req->uploadCallback && millis() - req->requestTime > 10000 &&
            fbdo->session.cfs.cbUploadInfo.status != fb_esp_cfs_upload_status_process_response)
        {
            req->requestTime = millis();
            fbdo->session.cfs.cbUploadInfo.status = fb_esp_cfs_upload_status_process_response;
            CFS_UploadStatusInfo in;
            in.status = fb_esp_cfs_upload_status_process_response;
            in.progress = 100;
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        }

        if (!fbdo->readResponse(&fbdo->session.cfs.payload, tcpHandler, response) && !response.isChunkedEnc)
        {
            complete = true;
            break;
        }

        // Last chunk?
        if (Utils::isChunkComplete(&tcpHandler, &response, complete))
            break;
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    // parse the payload for error
    fbdo->getError(fbdo->session.cfs.payload, tcpHandler, response, false);
    return tcpHandler.error.code == 0;
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

void FB_Firestore::sendUploadCallback(FirebaseData *fbdo, CFS_UploadStatusInfo &in,
                                      CFS_UploadProgressCallback cb, CFS_UploadStatusInfo *out)
{
    fbdo->session.cfs.cbUploadInfo = in;
    if (cb)
        cb(fbdo->session.cfs.cbUploadInfo);
    if (out)
        out = &fbdo->session.cfs.cbUploadInfo;
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