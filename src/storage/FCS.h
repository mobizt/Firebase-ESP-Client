/**
 * Google's Firebase Storage class, FCS.h version 1.0.6
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

#ifndef FB_Storage_H
#define FB_Storage_H

#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"

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
    bool upload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, const char *remotetFileName, const char *mime);

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
    bool upload(FirebaseData *fbdo, const char *bucketID, const uint8_t *data, size_t len, const char *remoteFileName, const char *mime);

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
    bool download(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType);

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
    bool getMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName);

    /** Delete file from Firebase Storage data bucket
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param bucketID The Firebase storage bucket ID in the project.
     * @param remotetFileName The file path includes its name of file in the data bucket.
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
    */
    bool deleteFile(FirebaseData *fbdo, const char *bucketID, const char *fileName);

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
    bool listFiles(FirebaseData *fbdo, const char *bucketID);

private:
    
    UtilsClass *ut = nullptr;
    void begin(UtilsClass *u);
    bool sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req);
    void rescon(FirebaseData *fbdo, const char *host);
    bool fcs_connect(FirebaseData *fbdo);
    bool fcs_sendRequest(FirebaseData *fbdo, struct fb_esp_fcs_req_t *req);
    bool handleResponse(FirebaseData *fbdo);
};

#endif