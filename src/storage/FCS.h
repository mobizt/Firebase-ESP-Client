
/**
 * Google's Firebase Storage class, FCS.h version 1.2.13
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

#ifndef FB_Storage_H
#define FB_Storage_H

#include "./FB_Utils.h"
#include "./session/FB_Session.h"

using namespace mb_string;

class FB_Storage
{
    friend class Firebase_ESP_Client;

public:
    struct RequestType;
    FB_Storage();
    ~FB_Storage();

    /** Upload file to the Firebase Storage data bucket.
     *
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param localFileName The file path includes its name to upload.
     * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
     * @param remotetFileName The file path includes its name of uploaded file in data bucket.
     * @param mime The file MIME type
     * @param callback Optional. The callback function that accept FCS_UploadStatusInfo data.
     * .
     * @return Boolean value, indicates the success of the operation.
     *
     * @note Use FirebaseData.downloadURL() to get the download link.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *>
    bool upload(FirebaseData *fbdo, T1 bucketID, T2 localFileName, firebase_mem_storage_type storageType,
                T3 remotetFileName, T4 mime, FCS_UploadProgressCallback callback = NULL)
    {
        return mUpload(fbdo, toStringPtr(bucketID), toStringPtr(localFileName), storageType,
                       toStringPtr(remotetFileName), toStringPtr(mime), callback);
    }

    /** Upload byte array to the Firebase Storage data bucket.
     *
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param data The byte array of data.
     * @param len The size of byte array data in bytes.
     * @param remotetFileName The file path includes its name of uploaded file in data bucket.
     * @param mime The file MIME type
     * @param callback Optional. The callback function that accept FCS_UploadStatusInfo data.
     * .
     * @return Boolean value, indicates the success of the operation.
     *
     * @note Use FirebaseData.downloadURL() to get the download link.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool upload(FirebaseData *fbdo, T1 bucketID, const uint8_t *data, size_t len, T2 remoteFileName,
                T3 mime, FCS_UploadProgressCallback callback = NULL)
    {
        return mUpload(fbdo, toStringPtr(bucketID), data, len, toStringPtr(remoteFileName), toStringPtr(mime), callback);
    }

    /** Download file from the Firebase Storage data bucket.
     *
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param remotetFileName The file path includes its name of file in the data bucket to download.
     * @param localFileName The file path includes its name to save.
     * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
     * @param callback Optional. The callback function that accept FCS_DownloadStatusInfo data.
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool download(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName, T3 localFileName,
                  firebase_mem_storage_type storageType, FCS_DownloadProgressCallback callback = NULL)
    {
        return mDownload(fbdo, toStringPtr(bucketID), toStringPtr(remoteFileName),
                         toStringPtr(localFileName), storageType, callback);
    }

    /** Download a firmware file from the Firebase Storage data bucket for OTA updates.
     *
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param remotetFileName The firmware file path includes its name of file in the data bucket to download.
     * @param callback Optional. The callback function that accept FCS_DownloadStatusInfo data.
     * @return Boolean value, indicates the success of the operation.
     *
     * @note: In ESP8266, this function will allocate 16k+ memory for internal SSL client.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool downloadOTA(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName, FCS_DownloadProgressCallback callback = NULL)
    {
        return mDownloadOTA(fbdo, toStringPtr(bucketID), toStringPtr(remoteFileName), callback);
    }

    /** Get the meta data of file in Firebase Storage data bucket
     *
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param remotetFileName The file path includes its name of file in the data bucket.
     *
     * @return Boolean value, indicates the success of the operation.
     *
     * @note Use the FileMetaInfo type data to get name, bucket, contentType, size,
     * generation, etag, crc32, downloadTokens properties from file.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool getMetadata(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName)
    {
        return mGetMetadata(fbdo, toStringPtr(bucketID), toStringPtr(remoteFileName));
    }

    /** Delete file from Firebase Storage data bucket
     *
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param remotetFileName The file path includes its name of file in the data bucket.
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool deleteFile(FirebaseData *fbdo, T1 bucketID, T2 fileName)
    {
        return mDeleteFile(fbdo, toStringPtr(bucketID), toStringPtr(fileName));
    }

    /** List all files in the Firebase Storage data bucket.
     *
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     *
     * @return Boolean value, indicates the success of the operation.
     *
     * @note Use the FileList type data to get name and bucket properties for each item.
     *
     */
    template <typename T = const char *>
    bool listFiles(FirebaseData *fbdo, T bucketID) { return mListFiles(fbdo, toStringPtr(bucketID)); }

private:
    bool sendRequest(FirebaseData *fbdo, struct firebase_fcs_req_t *req);
    void sendUploadCallback(FirebaseData *fbdo, FCS_UploadStatusInfo &in, FCS_UploadProgressCallback cb,
                            FCS_UploadStatusInfo *out);
    void sendDownloadCallback(FirebaseData *fbdo, FCS_DownloadStatusInfo &in,
                              FCS_DownloadProgressCallback cb, FCS_DownloadStatusInfo *out);
    void makeUploadStatus(FCS_UploadStatusInfo &info, const MB_String &local, const MB_String &remote,
                          firebase_fcs_upload_status status, size_t progress, size_t fileSize, int elapsedTime, const MB_String &msg);
    void makeDownloadStatus(FCS_DownloadStatusInfo &info, const MB_String &local, const MB_String &remote,
                            firebase_fcs_download_status status, size_t progress, size_t fileSize, int elapsedTime,
                            const MB_String &msg);
    void rescon(FirebaseData *fbdo, const char *host);
    bool fcs_connect(FirebaseData *fbdo);
    bool fcs_sendRequest(FirebaseData *fbdo, struct firebase_fcs_req_t *req);
    void reportUploadProgress(FirebaseData *fbdo, struct firebase_fcs_req_t *req, size_t readBytes);
    void reportDownloadProgress(FirebaseData *fbdo, struct firebase_fcs_req_t *req, size_t readBytes);
    bool handleResponse(FirebaseData *fbdo, struct firebase_fcs_req_t *req);
    bool mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr localFileName,
                 firebase_mem_storage_type storageType, MB_StringPtr remotetFileName, MB_StringPtr mime,
                 FCS_UploadProgressCallback callback = NULL);
    bool mUpload(FirebaseData *fbdo, MB_StringPtr bucketID, const uint8_t *data, size_t len,
                 MB_StringPtr remoteFileName, MB_StringPtr mime, FCS_UploadProgressCallback callback = NULL);
    bool mDownload(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName,
                   MB_StringPtr localFileName, firebase_mem_storage_type storageType, FCS_DownloadProgressCallback callback = NULL);
    bool mDownloadOTA(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName,
                      FCS_DownloadProgressCallback callback = NULL);
    bool mGetMetadata(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr remoteFileName);
    bool mDeleteFile(FirebaseData *fbdo, MB_StringPtr bucketID, MB_StringPtr fileName);
    bool mListFiles(FirebaseData *fbdo, MB_StringPtr bucketID);
    bool parseJsonResponse(FirebaseData *fbdo, PGM_P key_path);
};

#endif

#endif // ENABLE