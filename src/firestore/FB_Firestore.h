/**
 * Google's Cloud Firestore class, Forestore.h version 1.0.5
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

#ifndef _FB_FIRESTORE_H_
#define _FB_FIRESTORE_H_

#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"

class FB_Firestore
{
    friend class Firebase_ESP_Client;

public:
    FB_Firestore();
    ~FB_Firestore();

    /** Export the documents in the database to the Firebase Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param storagePath The path in the Firebase Storage data bucket to store the exported database.
     * @param collectionIds Which collection ids to export. Unspecified means all collections. Use comma (,) to separate between the collection ids.
     * .
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool exportDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds = "");

    /** Import the exported documents stored in the Firebase Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param storagePath The path in the Firebase Storage data bucket that stores the exported database.
     * @param collectionIds Which collection ids to import. Unspecified means all collections included in the import. Use comma (,) to separate between the collection ids.
     * .
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool importDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds = "");

    /** Create a document at the defined document path.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param documentPath The relative path of document to create in the collection.
     * @param content A Firestore document. See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document
     * @param mask The fields to return. If not set, returns all fields. Use comma (,) to separate between the field names.
     * .
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires Email/password, Custom token or OAuth2.0 authentication.
     * 
    */
    bool createDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *mask = "");

    /** Create a document in the defined collection id.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param collectionId The relative path of document collection id to create the document.
     * @param documentId The document id of document to be created.
     * @param content A Firestore document. See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document
     * @param mask The fields to return. If not set, returns all fields. Use comma (,) to separate between the field names.
     * .
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires Email/password, Custom token or OAuth2.0 authentication.
     * 
    */
    bool createDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, const char *documentId, const char *content, const char *mask = "");

    /** Patch or update a document at the defined path.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param documentPath The relative path of document to patch with the input document.
     * @param content A Firestore document. See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document
     * @param updateMask The fields to update. If the document exists on the server and has fields not referenced in the mask, they are left unchanged. 
     * Fields referenced in the mask, but not present in the input document (content), are deleted from the document on the server. 
     * Use comma (,) to separate between the field names.
     * @param mask The fields to return. If not set, returns all fields. If the document has a field that is not present in this mask, that field 
     * will not be returned in the response. Use comma (,) to separate between the field names.
     * @param exists When set to true, the target document must exist. When set to false, the target document must not exist.
     * @param updateTime When set, the target document must exist and have been last updated at that time. 
     * A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits. 
     * Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires Email/password, Custom token or OAuth2.0 authentication.
     * 
    */
    bool patchDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *updateMask, const char *mask = "", const char *exists = "", const char *updateTime = "");

    /** Get a document at the defined path.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param documentPath The relative path of document to get.
     * @param mask The fields to return. If not set, returns all fields. If the document has a field that is not present in this mask,
     * that field will not be returned in the response. Use comma (,) to separate between the field names.
     * @param transaction Reads the document in a transaction. A base64-encoded string.
     * @param readTime Reads the version of the document at the given time. This may not be older than 270 seconds.
     * A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits. 
     * Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires Email/password, Custom token or OAuth2.0 authentication.
     * 
    */
    bool getDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *mask = "", const char *transaction = "", const char *readTime = "");

    /** Runs a query.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param documentPath The relative path of document to get.
     * @param structuredQuery The pointer to FirebaseJson object that contains the Firestore query. For the description of structuredQuery, see https://cloud.google.com/firestore/docs/reference/rest/v1/StructuredQuery
     * @param consistencyMode Optional. The consistency mode for this transaction e.g. fb_esp_firestore_consistency_mode_transaction,fb_esp_firestore_consistency_mode_newTransaction
     * and fb_esp_firestore_consistency_mode_readTime
     * @param consistency Optional. The value based on consistency mode e.g. transaction string, TransactionOptions (JSON) and date time string.
     * 
     * For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.documents/runQuery#body.request_body.FIELDS
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * 
    */
    bool runQuery(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, FirebaseJson *structuredQuery, fb_esp_firestore_consistency_mode consistencyMode = fb_esp_firestore_consistency_mode_undefined, const char *consistency = "");

    /** Delete a document at the defined path.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param documentPath The relative path of document to delete.
     * @param exists When set to true, the target document must exist. When set to false, the target document must not exist.
     * @param updateTime When set, the target document must exist and have been last updated at that time.
     * A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits.
     * Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * 
     * This function requires Email/password, Custom token or OAuth2.0 authentication.
     * 
    */
    bool deleteDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *exists = "", const char *updateTime = "");

    /** List the documents in the defined documents collection.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param collectionId The relative path of document colection.
     * @param pageSize The maximum number of documents to return.
     * @param pageToken The nextPageToken value returned from a previous List request, if any.
     * @param orderBy The order to sort results by. For example: priority desc, name.
     * @param mask The fields to return. If not set, returns all fields.
     * If a document has a field that is not present in this mask, that field will not be returned in the response.
     * @param showMissing If the list should show missing documents. 
     * A missing document is a document that does not exist but has sub-documents. 
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires Email/password, Custom token or OAuth2.0 authentication (when showMissing is true).
     * 
    */
    bool listDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, int pageSize, const char *pageToken, const char *orderBy, const char *mask, bool showMissing);

    /** List the document collection ids in the defined document path.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param databaseId The Firebase Cloud Firestore database id which is (default) or empty "".
     * @param documentPath The relative path of document to get its collections' id.
     * @param pageSize The maximum number of results to return.
     * @param pageToken The nextPageToken value returned from a previous List request, if any.
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
    */
    bool listCollectionIds(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, int pageSize, const char *pageToken);

private:
    UtilsClass *ut = nullptr;
    void begin(UtilsClass *u);
    void rescon(FirebaseData *fbdo, const char *host);
    bool connect(FirebaseData *fbdo);
    bool sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req);
    bool firestore_sendRequest(FirebaseData *fbdo, struct fb_esp_firestore_req_t *req);
    bool handleResponse(FirebaseData *fbdo);
};

#endif