/**
 * Google's Cloud Storage class, GCS.h version 1.1.8
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 1, 2022
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

#ifdef ENABLE_GC_STORAGE

#ifndef GOOGLE_CS_H
#define GOOGLE_CS_H

#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"

using namespace mb_string;

class GG_CloudStorage
{
    friend class Firebase_ESP_Client;

public:
    struct RequestType;
    GG_CloudStorage();
    ~GG_CloudStorage();

    /** Upload file to the Google Cloud Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase or Google Cloud Storage bucket ID.
     * @param localFileName The file path includes its name to upload.
     * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
     * @param uploadType The enum of type of upload methods e.g. gcs_upload_type_simple, gcs_upload_type_multipart, gcs_upload_type_resumable
     * @param remotetFileName The file path includes its name of uploaded file in data bucket.
     * @param mime The file MIME type.
     * @param uploadOptions Optional. The UploadOptions data contains the query parameters options.
     * For query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters
     * @param requestProps Optional. The RequestProperties data contains the request payload properties.
     * For request payload properties, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties
     * @param status Optional. The UploadStatusInfo data to get the upload status.
     * @param callback Optional. The callback function that accept UploadStatusInfo data.
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note This function requires OAuth2.0 authentication.
     * 
     * The upload types of methods can be selectable.
     * The gcs_upload_type_simple upload type is used for small file upload in a single request without metadata.
     * gcs_upload_type_multipart upload type is for small file upload in a single reques with metadata.
     * gcs_upload_type_resumable upload type is for medium or large file (larger than or equal to 256 256 KiB) upload with metadata and can be resumable.
     * 
     * The upload with metadata supports allows the library to add the metadata internally for Firebase to request the download access token in Firebase Storage bucket.
     * User also can add custom metadata for the uploading file (object).
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *>
    bool upload(FirebaseData *fbdo, T1 bucketID, T2 localFileName, fb_esp_mem_storage_type storageType, fb_esp_gcs_upload_type uploadType, T3 remoteFileName, T4 mime, UploadOptions *uploadOptions = nullptr, RequestProperties *requestProps = nullptr, UploadStatusInfo *status = nullptr, ProgressCallback callback = NULL) { return mUpload(fbdo, toString(bucketID), toString(localFileName), storageType, uploadType, toString(remoteFileName), toString(mime), uploadOptions, requestProps, status, callback); }

    /** Downoad file from the Google Cloud Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase or Google Cloud Storage bucket ID.
     * @param remotetFileName The file path includes its name of file in the data bucket to download.
     * @param localFileName The file path includes its name to save.
     * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
     * @param options Optional. The pointer to StorageGetOptions data that contains the get query parameters.
     * For the query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note This function requires OAuth2.0 authentication.
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool download(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName, T3 localFileName, fb_esp_mem_storage_type storageType, StorageGetOptions *options = nullptr) { return mDownload(fbdo, toString(bucketID),  toString(remoteFileName), toString(localFileName), storageType, options); }

    /** Download a firmware file from the Google Cloud Storage data bucket for OTA updates.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase or Google Cloud Storage bucket ID.
     * @param remotetFileName The firmware file path includes its name of file in the data bucket to download.
     * @return Boolean value, indicates the success of the operation.
     * 
     * @note: In ESP8266, this function will allocate 16k+ memory for internal SSL client.
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *>
    bool downloadOTA(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName) { return mDownloadOTA(fbdo, toString(bucketID), toString(remoteFileName)); }

    /** Get the meta data of file in Firebase or Google Cloud Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase or Google Cloud Storage bucket ID.
     * @param remotetFileName The file path includes its name of file in the data bucket.
     * @param options Optional. The pointer to StorageGetOptions data that contains the get query parameters.
     * For the query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use the FileMetaInfo type data to get name, bucket, contentType, size, 
     * generation, etag, crc32, downloadTokens properties from file.
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *>
    bool getMetadata(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName, StorageGetOptions *options = nullptr) { return mGetMetadata(fbdo, toString(bucketID), toString(remoteFileName), options); }

    /** Delete file from Firebase or Google Cloud Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase or Google Cloud Storage bucket ID.
     * @param remotetFileName The file path includes its name of file in the data bucket.
     * @param options Optional. The pointer to DeleteOptions data contains the query parameters.
     * For query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/delete#optional-parameters
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *>
    bool deleteFile(FirebaseData *fbdo, T1 bucketID, T2 fileName, DeleteOptions *options = nullptr) { return mDeleteFile(fbdo, toString(bucketID), toString(fileName), options); }

    /** List all files in the Firebase or Google Cloud Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase or Google Cloud Storage bucket ID.
     * @param options Optional. The pointer to ListOptions data that contains the query parameters
     * Fore query parameters description, see https://cloud.google.com/storage/docs/json_api/v1/objects/list#optional-parameters
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use the FileList type data to get name and bucket properties for each item.
     * 
    */
    template <typename T = const char *>
    bool listFiles(FirebaseData *fbdo, T bucketID, ListOptions *options = nullptr) { return mListFiles(fbdo, toString(bucketID), options); }

private:
    const uint32_t gcs_min_chunkSize = 256 * 1024; //Min Google recommended length
    uint32_t gcs_chunkSize = 256 * 1024;
    bool _resumable_upload_task_enable = false;
    UtilsClass *ut = nullptr;
    std::vector<struct fb_gcs_upload_resumable_task_info_t> _resumableUploadTasks = std::vector<struct fb_gcs_upload_resumable_task_info_t>();
    size_t _resumableUplaodTaskIndex = 0;

    void begin(UtilsClass *u);
    void rescon(FirebaseData *fbdo, const char *host);
    void setGetOptions(struct fb_esp_gcs_req_t *req, MBSTRING &header, bool hasParams);
    void setListOptions(struct fb_esp_gcs_req_t *req, MBSTRING &header, bool hasParams);
    void setUploadOptions(struct fb_esp_gcs_req_t *req, MBSTRING &header, bool hasParams);
    void setDeleteOptions(struct fb_esp_gcs_req_t *req, MBSTRING &header, bool hasParams);
    void reportUpploadProgress(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req, size_t readBytes);
    void setRequestproperties(struct fb_esp_gcs_req_t *req, FirebaseJson *json, bool &hasProps);
    void sendCallback(FirebaseData *fbdo, UploadStatusInfo &in, ProgressCallback cb, UploadStatusInfo *out);
    bool sendRequest(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req);
    bool gcs_connect(FirebaseData *fbdo);
    bool gcs_sendRequest(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req);
    bool handleResponse(FirebaseData *fbdo, struct fb_esp_gcs_req_t *req);
    bool mUpload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, fb_esp_gcs_upload_type uploadType, const char *remoteFileName, const char *mime, UploadOptions *uploadOptions = nullptr, RequestProperties *requestProps = nullptr, UploadStatusInfo *status = nullptr, ProgressCallback callback = NULL);
    bool mDownload(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType, StorageGetOptions *options = nullptr);
    bool mDownloadOTA(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName);
    bool mGetMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, StorageGetOptions *options = nullptr);
    bool mDeleteFile(FirebaseData *fbdo, const char *bucketID, const char *fileName, DeleteOptions *options = nullptr);
    bool mListFiles(FirebaseData *fbdo, const char *bucketID, ListOptions *options = nullptr);

#if defined(ESP32)
    void runResumableUploadTask(const char *taskName);
#elif defined(ESP8266)
    void runResumableUploadTask();
#endif

protected:
    template <typename T>
    auto toString(const T &val) -> typename enable_if<is_std_string<T>::value || is_arduino_string<T>::value || is_mb_string<T>::value || is_same<T, StringSumHelper>::value, const char *>::type { return val.c_str(); }

    template <typename T>
    auto toString(T val) -> typename enable_if<is_const_chars<T>::value, const char *>::type { return val; }

    template <typename T>
    auto toString(T val) -> typename enable_if<fs_t<T>::value, const char *>::type { return (const char *)val; }

    template <typename T>
    auto toString(T val) -> typename enable_if<is_same<T, std::nullptr_t>::value, const char *>::type { return ""; }
};

#endif

#endif //ENABLE