/**
 * Google's Firebase Storage class, FCS.h version 1.1.11
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

#ifdef ENABLE_FB_STORAGE

#ifndef FB_Storage_H
#define FB_Storage_H

#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"

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
     * .
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.downloadURL() to get the download link.
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *>
    bool upload(FirebaseData *fbdo, T1 bucketID, T2 localFileName, fb_esp_mem_storage_type storageType, T3 remotetFileName, T4 mime) { return mUpload(fbdo, toString(bucketID), toString(localFileName), storageType, toString(remotetFileName), toString(mime)); }

    /** Upload byte array to the Firebase Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param data The byte array of data.
     * @param len The size of byte array data in bytes.
     * @param remotetFileName The file path includes its name of uploaded file in data bucket.
     * @param mime The file MIME type
     * .
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.downloadURL() to get the download link.
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool upload(FirebaseData *fbdo, T1 bucketID, const uint8_t *data, size_t len, T2 remoteFileName, T3 mime) { return mUpload(fbdo, toString(bucketID), data, len, toString(remoteFileName), toString(mime)); }

    /** Download file from the Firebase Storage data bucket.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param remotetFileName The file path includes its name of file in the data bucket to download.
     * @param localFileName The file path includes its name to save.
     * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool download(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName, T3 localFileName, fb_esp_mem_storage_type storageType) { return mDownload(fbdo, toString(bucketID), toString(remoteFileName), toString(localFileName), storageType); }

    /** Download a firmware file from the Firebase Storage data bucket for OTA updates.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param remotetFileName The firmware file path includes its name of file in the data bucket to download.
     * @return Boolean value, indicates the success of the operation.
     * 
     * @note: In ESP8266, this function will allocate 16k+ memory for internal SSL client.
     * 
    */
    template <typename T1 = const char *, typename T2 = const char *>
    bool downloadOTA(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName) { return mDownloadOTA(fbdo, toString(bucketID), toString(remoteFileName)); }

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
    bool getMetadata(FirebaseData *fbdo, T1 bucketID, T2 remoteFileName) { return mGetMetadata(fbdo, toString(bucketID), toString(remoteFileName)); }

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
    bool deleteFile(FirebaseData *fbdo, T1 bucketID, T2 fileName) { return mDeleteFile(fbdo, toString(bucketID), toString(fileName)); }

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
    bool listFiles(FirebaseData *fbdo, T bucketID) { return mListFiles(fbdo, toString(bucketID)); }

private:
    UtilsClass *ut = nullptr;
    void begin(UtilsClass *u);
    bool sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req);
    void rescon(FirebaseData *fbdo, const char *host);
    bool fcs_connect(FirebaseData *fbdo);
    bool fcs_sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req);
    bool handleResponse(FirebaseData *fbdo);
    bool mUpload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, const char *remotetFileName, const char *mime);
    bool mUpload(FirebaseData *fbdo, const char *bucketID, const uint8_t *data, size_t len, const char *remoteFileName, const char *mime);
    bool mDownload(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType);
    bool mDownloadOTA(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName);
    bool mGetMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName);
    bool mDeleteFile(FirebaseData *fbdo, const char *bucketID, const char *fileName);
    bool mListFiles(FirebaseData *fbdo, const char *bucketID);

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