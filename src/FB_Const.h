#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
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

#ifndef FB_CONST_H_
#define FB_CONST_H_

#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#include <time.h>

#if !defined(__AVR__)
#include <vector>
#include <functional>
#endif

#include "FB_Network.h"
#include "FirebaseFS.h"
#include "./mbfs/MB_FS.h"

#if defined(FIREBASE_USE_PSRAM)
#define FIREBASEJSON_USE_PSRAM
#endif
#include "json/FirebaseJson.h"
#include "MB_NTP.h"

#if defined(ENABLE_OTA_FIRMWARE_UPDATE) && (defined(ENABLE_RTDB) || defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE))
#if defined(ESP32)
#include <Update.h>
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO)
#include <Updater.h>
#endif
#define OTA_UPDATE_ENABLED
#endif

#if defined(ESP32)
#if defined(ESP_ARDUINO_VERSION)
#if ESP_ARDUINO_VERSION > ESP_ARDUINO_VERSION_VAL(2, 0, 1)
#define ESP32_GT_2_0_1_FS_MEMORY_FIX
#endif
#endif
#endif

#if defined(FIREBASE_ESP_CLIENT)

#define FIREBASE_STREAM_CLASS FirebaseStream
#define FIREBASE_MP_STREAM_CLASS MultiPathStream
#define FIREBASE_CLASS Firebase_ESP_Client

#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)

#define FIREBASE_STREAM_CLASS StreamData
#define FIREBASE_MP_STREAM_CLASS MultiPathStreamData
#if defined(ESP32)
#define FIREBASE_CLASS FirebaseESP32
#elif defined(ES8266) || defined(MB_ARDUINO_PICO)
#define FIREBASE_CLASS FirebaseESP8266
#endif

#endif

class FirebaseData;
class PolicyInfo;
class FunctionsConfig;
class QueryFilter;

#define FIREBASE_PORT 443

#define MAX_REDIRECT 5

#define MIN_WIFI_RECONNECT_TIMEOUT 10 * 1000
#define MAX_WIFI_RECONNECT_TIMEOUT 5 * 60 * 1000

#define MIN_SOCKET_CONN_TIMEOUT 1 * 1000
#define DEFAULT_SOCKET_CONN_TIMEOUT 10 * 1000
#define MAX_SOCKET_CONN_TIMEOUT 60 * 1000

#define MIN_SERVER_RESPONSE_TIMEOUT 1 * 1000
#define DEFAULT_SERVER_RESPONSE_TIMEOUT 10 * 1000
#define MAX_SERVER_RESPONSE_TIMEOUT 60 * 1000

#define MIN_RTDB_KEEP_ALIVE_TIMEOUT 20 * 1000
#define DEFAULT_RTDB_KEEP_ALIVE_TIMEOUT 45 * 1000
#define MAX_RTDB_KEEP_ALIVE_TIMEOUT 2 * 60 * 1000

#define MIN_RTDB_STREAM_RECONNECT_INTERVAL 1000
#define MAX_RTDB_STREAM_RECONNECT_INTERVAL 60 * 1000

#define MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL 3 * 1000
#define MAX_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL 30 * 1000

#define MIN_TOKEN_GENERATION_BEGIN_STEP_INTERVAL 300

#define MIN_TOKEN_GENERATION_ERROR_INTERVAL 5 * 1000

#define MIN_NTP_SERVER_SYNC_TIME_OUT 15 * 1000

#define FB_TIME_SYNC_INTERVAL 1500

#define DEFAULT_AUTH_TOKEN_PRE_REFRESH_SECONDS 5 * 60

#define DEFAULT_AUTH_TOKEN_EXPIRED_SECONDS 3600

#define DEFAULT_REQUEST_TIMEOUT 2000

// The TCP session will be closed when time out reached
#define DEFAULT_TCP_CONNECTION_TIMEOUT 3 * 60 * 1000

#define SD_CS_PIN 15

#define STREAM_TASK_STACK_SIZE 8192
#define QUEUE_TASK_STACK_SIZE 8192
#define MAX_BLOB_PAYLOAD_SIZE 1024
#define ESP_DEFAULT_TS 1618971013
#define ESP_REPORT_PROGRESS_INTERVAL 2

#define _NO_REF 0
#define _NO_QUERY 0
#define _NO_SUB_TYPE 0
#define _NO_BLOB_SIZE 0
#define _NO_PRIORITY 0
#define _NO_PAYLOAD ""
#define _NO_ETAG ""
#define _NO_FILE ""
#define _EMPTY_STR ""
#define _IS_ASYNC true
#define _NO_ASYNC false
#define _NO_QUEUE false

#include "FB_Error.h"

class FirebaseData;

typedef void (*FB_TCPConnectionRequestCallback)(const char *, int);
typedef void (*FB_NetworkConnectionRequestCallback)(void);
typedef void (*FB_NetworkStatusRequestCallback)(void);
typedef void (*FB_ResponseCallback)(const char *);

typedef enum
{
    mem_storage_type_undefined,
    mem_storage_type_flash,
    mem_storage_type_sd
} fb_esp_mem_storage_type;

#if !defined(FIREBASE_ESP_CLIENT)
struct StorageType
{
    static const int8_t UNDEFINED = 0; // add to compatible with fb_esp_mem_storage_type enum
    static const int8_t FLASH = 1;     // now set to 1 instead of 0 in older version
    static const int8_t SD = 2;        // now set to 2 instead of 1 in older version
};
#endif

enum fb_esp_con_mode
{
    fb_esp_con_mode_undefined,
    fb_esp_con_mode_rtdb,
    fb_esp_con_mode_rtdb_stream,
    fb_esp_con_mode_fcm,
    fb_esp_con_mode_storage,
    fb_esp_con_mode_gc_storage,
    fb_esp_con_mode_firestore,
    fb_esp_con_mode_functions
};

enum fb_esp_data_type
{
    d_any = 0,
    d_null,
    d_integer,
    d_float,
    d_double,
    d_boolean,
    d_string,
    d_json,
    d_array,
    d_blob,
    d_file,
    d_file_ota,
    d_timestamp,
    d_shallow
};

enum fb_esp_rtdb_data_type
{
    fb_esp_rtdb_data_type_null = d_null,
    fb_esp_rtdb_data_type_integer,
    fb_esp_rtdb_data_type_float,
    fb_esp_rtdb_data_type_double,
    fb_esp_rtdb_data_type_boolean,
    fb_esp_rtdb_data_type_string,
    fb_esp_rtdb_data_type_json,
    fb_esp_rtdb_data_type_array,
    fb_esp_rtdb_data_type_blob,
    fb_esp_rtdb_data_type_file
};

enum fb_esp_request_method
{
    http_undefined,
    http_put,
    http_post,
    http_get,
    http_patch,
    http_delete,

    rtdb_set_nocontent,    /* HTTP PUT (No content) */
    rtdb_update_nocontent, /* HTTP PATCH (No content) */
    rtdb_get_nocontent,    /* HTTP GET (No content) */
    rtdb_stream,           /* HTTP GET (SSE) */
    rtdb_backup,           /* HTTP GET */
    rtdb_restore,          /* HTTP PATCH */
    rtdb_get_rules,        /* HTTP GET */
    rtdb_set_rules,        /* HTTP PUT */
    rtdb_get_shallow,      /* HTTP GET */
    rtdb_get_priority,     /* HTTP GET */
    rtdb_set_priority,     /* HTTP PUT */
};

enum fb_esp_http_connection_type
{
    fb_esp_http_connection_type_undefined,
    fb_esp_http_connection_type_keep_alive,
    fb_esp_http_connection_type_close
};

enum fb_esp_settings_provider_type
{
    auth_provider_type_login,
    auth_provider_type_service_account
};

enum fb_esp_auth_token_status
{
    token_status_uninitialized,
    token_status_on_initialize,
    token_status_on_signing,
    token_status_on_request,
    token_status_on_refresh,
    token_status_ready,
    token_status_error
};

enum fb_esp_auth_token_type
{
    token_type_undefined,
    token_type_legacy_token,
    token_type_id_token,
    token_type_custom_token,
    token_type_oauth2_access_token,
    token_type_refresh_token
};

enum fb_esp_jwt_generation_step
{
    fb_esp_jwt_generation_step_begin,
    fb_esp_jwt_generation_step_encode_header_payload,
    fb_esp_jwt_generation_step_sign,
    fb_esp_jwt_generation_step_exchange
};

enum fb_esp_user_email_sending_type
{
    fb_esp_user_email_sending_type_verify,
    fb_esp_user_email_sending_type_reset_psw
};

#ifdef ENABLE_RTDB

enum fb_esp_rtdb_task_type
{
    fb_esp_rtdb_task_undefined,
    fb_esp_rtdb_task_download_rules,
    fb_esp_rtdb_task_read_rules,
    fb_esp_rtdb_task_upload_rules,
    fb_esp_rtdb_task_store_rules
};

enum fb_esp_rtdb_value_type
{
    fb_esp_rtdb_value_type_any = 0,
    fb_esp_rtdb_value_type_std_string,
    fb_esp_rtdb_value_type_arduino_string,
    fb_esp_rtdb_value_type_mb_string,
    fb_esp_rtdb_value_type_char_array,
    fb_esp_rtdb_value_type_int,
    fb_esp_rtdb_value_type_float,
    fb_esp_rtdb_value_type_double,
    fb_esp_rtdb_value_type_json,
    fb_esp_rtdb_value_type_array,
    fb_esp_rtdb_value_type_blob
};

enum fb_esp_rtdb_upload_status
{
    fb_esp_rtdb_upload_status_error = -1,
    fb_esp_rtdb_upload_status_unknown = 0,
    fb_esp_rtdb_upload_status_init,
    fb_esp_rtdb_upload_status_upload,
    fb_esp_rtdb_upload_status_complete
};

enum fb_esp_rtdb_download_status
{
    fb_esp_rtdb_download_status_error = -1,
    fb_esp_rtdb_download_status_unknown = 0,
    fb_esp_rtdb_download_status_init,
    fb_esp_rtdb_download_status_download,
    fb_esp_rtdb_download_status_complete
};

#endif

#if defined(FIREBASE_ESP_CLIENT)

#if defined(ENABLE_FIRESTORE)
enum fb_esp_cfs_upload_status
{
    fb_esp_cfs_upload_status_error = -1,
    fb_esp_cfs_upload_status_unknown = 0,
    fb_esp_cfs_upload_status_init,
    fb_esp_cfs_upload_status_upload,
    fb_esp_cfs_upload_status_complete,
    fb_esp_cfs_upload_status_process_response
};
#endif

#ifdef ENABLE_FCM
enum fb_esp_fcm_msg_mode
{
    fb_esp_fcm_msg_mode_legacy_http,
    fb_esp_fcm_msg_mode_httpv1,
    fb_esp_fcm_msg_mode_subscribe,
    fb_esp_fcm_msg_mode_unsubscribe,
    fb_esp_fcm_msg_mode_app_instance_info,
    fb_esp_fcm_msg_mode_apn_token_registration
};
#endif

#ifdef ENABLE_GC_STORAGE
enum fb_esp_gcs_request_type
{
    fb_esp_gcs_request_type_undefined,
    fb_esp_gcs_request_type_upload_simple,
    fb_esp_gcs_request_type_upload_multipart,
    fb_esp_gcs_request_type_upload_resumable_init,
    fb_esp_gcs_request_type_upload_resumable_run,
    fb_esp_gcs_request_type_download,
    fb_esp_gcs_request_type_patch,
    fb_esp_gcs_request_type_delete,
    fb_esp_gcs_request_type_get_metadata,
    fb_esp_gcs_request_type_set_metadata,
    fb_esp_gcs_request_type_update_metadata,
    fb_esp_gcs_request_type_list,
    fb_esp_gcs_request_type_download_ota
};

enum fb_esp_gcs_upload_type
{
    gcs_upload_type_simple,
    gcs_upload_type_multipart,
    gcs_upload_type_resumable
};

enum fb_esp_gcs_upload_status
{
    fb_esp_gcs_upload_status_error = -1,
    fb_esp_gcs_upload_status_unknown = 0,
    fb_esp_gcs_upload_status_init,
    fb_esp_gcs_upload_status_upload,
    fb_esp_gcs_upload_status_complete
};

enum fb_esp_gcs_download_status
{
    fb_esp_gcs_download_status_error = -1,
    fb_esp_gcs_download_status_unknown = 0,
    fb_esp_gcs_download_status_init,
    fb_esp_gcs_download_status_download,
    fb_esp_gcs_download_status_complete
};

#endif

#ifdef ENABLE_FB_STORAGE
enum fb_esp_fcs_request_type
{
    fb_esp_fcs_request_type_undefined,
    fb_esp_fcs_request_type_upload,
    fb_esp_fcs_request_type_upload_pgm_data,
    fb_esp_fcs_request_type_download,
    fb_esp_fcs_request_type_get_meta,
    fb_esp_fcs_request_type_delete,
    fb_esp_fcs_request_type_list,
    fb_esp_fcs_request_type_download_ota
};

enum fb_esp_fcs_upload_status
{
    fb_esp_fcs_upload_status_error = -1,
    fb_esp_fcs_upload_status_unknown = 0,
    fb_esp_fcs_upload_status_init,
    fb_esp_fcs_upload_status_upload,
    fb_esp_fcs_upload_status_complete
};

enum fb_esp_fcs_download_status
{
    fb_esp_fcs_download_status_error = -1,
    fb_esp_fcs_download_status_unknown = 0,
    fb_esp_fcs_download_status_init,
    fb_esp_fcs_download_status_download,
    fb_esp_fcs_download_status_complete
};
#endif

#ifdef ENABLE_FIRESTORE
enum fb_esp_firestore_request_type
{
    fb_esp_firestore_request_type_undefined,
    fb_esp_firestore_request_type_export_docs,
    fb_esp_firestore_request_type_import_docs,
    fb_esp_firestore_request_type_get_doc,
    fb_esp_firestore_request_type_batch_get_doc,
    fb_esp_firestore_request_type_create_doc,
    fb_esp_firestore_request_type_patch_doc,
    fb_esp_firestore_request_type_delete_doc,
    fb_esp_firestore_request_type_run_query,
    fb_esp_firestore_request_type_begin_transaction,
    fb_esp_firestore_request_type_rollback,
    fb_esp_firestore_request_type_list_doc,
    fb_esp_firestore_request_type_list_collection,
    fb_esp_firestore_request_type_commit_document,
    fb_esp_firestore_request_type_batch_write_doc,
    fb_esp_firestore_request_type_create_index,
    fb_esp_firestore_request_type_delete_index,
    fb_esp_firestore_request_type_list_index,
    fb_esp_firestore_request_type_get_index
};

enum fb_esp_firestore_document_write_type
{
    fb_esp_firestore_document_write_type_undefined,
    fb_esp_firestore_document_write_type_update,
    fb_esp_firestore_document_write_type_delete,
    fb_esp_firestore_document_write_type_transform
};

enum fb_esp_firestore_transform_type
{
    fb_esp_firestore_transform_type_undefined,
    fb_esp_firestore_transform_type_set_to_server_value,
    fb_esp_firestore_transform_type_increment,
    fb_esp_firestore_transform_type_maaximum,
    fb_esp_firestore_transform_type_minimum,
    fb_esp_firestore_transform_type_append_missing_elements,
    fb_esp_firestore_transform_type_remove_all_from_array
};

enum fb_esp_firestore_consistency_mode
{
    fb_esp_firestore_consistency_mode_undefined,
    fb_esp_firestore_consistency_mode_transaction,
    fb_esp_firestore_consistency_mode_newTransaction,
    fb_esp_firestore_consistency_mode_readTime
};

#endif

#ifdef ENABLE_FB_FUNCTIONS
enum fb_esp_functions_creation_step
{
    fb_esp_functions_creation_step_idle,
    fb_esp_functions_creation_step_gen_upload_url,
    fb_esp_functions_creation_step_upload_zip_file,
    fb_esp_functions_creation_step_upload_source_files,
    fb_esp_functions_creation_step_deploy,
    fb_esp_functions_creation_step_polling_status,
    fb_esp_functions_creation_step_set_iam_policy,
    fb_esp_functions_creation_step_delete
};

enum fb_esp_functions_request_type
{
    fb_esp_functions_request_type_undefined,
    fb_esp_functions_request_type_call,
    fb_esp_functions_request_type_create,
    fb_esp_functions_request_type_delete,
    fb_esp_functions_request_type_gen_download_url,
    fb_esp_functions_request_type_gen_upload_url,
    fb_esp_functions_request_type_upload,
    fb_esp_functions_request_type_pgm_upload,
    fb_esp_functions_request_type_upload_bucket_sources,
    fb_esp_functions_request_type_get,
    fb_esp_functions_request_type_get_iam_policy,
    fb_esp_functions_request_type_list,
    fb_esp_functions_request_type_patch,
    fb_esp_functions_request_type_set_iam_policy,
    fb_esp_functions_request_type_test_iam_policy,
    fb_esp_functions_request_type_list_operations,
};

enum fb_esp_functions_sources_type
{
    functions_sources_type_undefined,
    functions_sources_type_storage_bucket_archive,
    functions_sources_type_storage_bucket_sources,
    functions_sources_type_flash_data,
    functions_sources_type_local_archive,
    functions_sources_type_repository,

};

enum fb_esp_functions_trigger_type
{
    fb_esp_functions_trigger_type_undefined,
    fb_esp_functions_trigger_type_https,
    fb_esp_functions_trigger_type_event
};

enum fb_esp_functions_status
{
    fb_esp_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED,
    fb_esp_functions_status_ACTIVE,
    fb_esp_functions_status_OFFLINE,
    fb_esp_functions_status_DEPLOY_IN_PROGRESS,
    fb_esp_functions_status_DELETE_IN_PROGRESS,
    fb_esp_functions_status_UNKNOWN
};

enum fb_esp_functions_operation_status
{
    fb_esp_functions_operation_status_unknown,
    fb_esp_functions_operation_status_generate_upload_url,
    fb_esp_functions_operation_status_upload_source_file_in_progress,
    fb_esp_functions_operation_status_deploy_in_progress,
    fb_esp_functions_operation_status_set_iam_policy_in_progress,
    fb_esp_functions_operation_status_delete_in_progress,
    fb_esp_functions_operation_status_finished,
    fb_esp_functions_operation_status_error
};

#endif

#endif

struct fb_esp_wifi_credential_t
{
    MB_String ssid;
    MB_String password;
};

struct fb_esp_wifi_credentials_t
{
    friend class Firebase_Signer;

public:
    fb_esp_wifi_credentials_t(){};
    ~fb_esp_wifi_credentials_t() { clearAP(); };
    void addAP(const String &ssid, const String &password)
    {
        fb_esp_wifi_credential_t cred;
        cred.ssid = ssid;
        cred.password = password;
        credentials.push_back(cred);
    }
    void clearAP()
    {
        credentials.clear();
    }
    size_t size()
    {
        return credentials.size();
    }

    fb_esp_wifi_credential_t operator[](size_t index)
    {
        return credentials[index];
    }

private:
    MB_List<fb_esp_wifi_credential_t> credentials;
};

class fb_esp_wifi
{
    friend class Firebase_Signer;

public:
    fb_esp_wifi(){};
    ~fb_esp_wifi()
    {
        wifiCreds.clearAP();
    };
    void addAP(const String &ssid, const String &password)
    {
        wifiCreds.addAP(ssid, password);
    }
    void clearAP()
    {
        wifiCreds.clearAP();
    }

private:
    fb_esp_wifi_credentials_t wifiCreds;
};

template <typename T>
struct fb_esp_base64_io_t
{
    // the total bytes of data in output buffer
    int bufWrite = 0;
    // the size of output buffer
    size_t bufLen = 1024;
    // for file, the type of filesystem to write
    mbfs_file_type filetype = mb_fs_mem_storage_type_undefined;
    // for T array
    T *outT = nullptr;
    // for T vector
    MB_VECTOR<T> *outL = nullptr;
    // for client
    Client *outC = nullptr;
    // for ota
    bool ota = false;
};

struct fb_esp_response_t
{
    int code = 0;
};

struct fb_esp_auth_token_error_t
{
    MB_String message;
    int code = 0;
};

struct server_response_data_t
{
    int httpCode = 0;
    // Must not be negative
    int payloadLen = 0;
    // The response content length, must not be negative as it uses to determine
    // the available data to read in event-stream
    // and content length specific read in http response
    int contentLen = 0;
    int chunkRange = 0;
    fb_esp_data_type dataType = fb_esp_data_type::d_any;
    int payloadOfs = 0;
    bool boolData = false;
    int intData = 0;
    float floatData = 0.0f;
    double doubleData = 0.0f;
    MB_VECTOR<uint8_t> blobData;
    bool isEvent = false;
    bool noEvent = false;
    bool isChunkedEnc = false;
    bool hasEventData = false;
    bool noContent = false;
    bool eventPathChanged = false;
    bool dataChanged = false;
    bool redirect = false;
    MB_String location;
    MB_String contentType;
    MB_String connection;
    MB_String eventPath;
    MB_String eventType;
    MB_String eventData;
    MB_String etag;
    MB_String pushName;
    MB_String fbError;
    MB_String transferEnc;
};

struct fb_esp_chunk_state_info
{
    int state = 0;
    int chunkedSize = 0;
    int dataLen = 0;
};

struct fb_esp_tcp_response_handler_t
{
    // the chunk index of all data that is being process
    int chunkIdx = 0;
    // the payload chunk index that is being process
    int pChunkIdx = 0;
    // the total bytes of http response payload to read
    int payloadLen = 0;
    // the total bytes of base64 decoded data from response payload
    int decodedPayloadLen = 0;
    // the current size of chunk data to read from client
    int chunkBufSize = 0;
    // the amount of http response payload that read,
    // compare with the content length header value for finishing check
    int payloadRead = 0;
    // status showed that the http headers was found and is being read
    bool isHeader = false;
    // status showed that the http headers was completely read
    bool headerEnded = false;
    // status for OTA request
    bool downloadOTA = false;
    // the prefered size of chunk data to read from client
    size_t defaultChunkSize = 0;
    // keep the auth token generation error
    struct fb_esp_auth_token_error_t error;
    // keep the http header or the first line of stream event data
    MB_String header;
    // time out checking for execution
    unsigned long dataTime = 0;
    // pointer to payload
    MB_String *payload = nullptr;
    // data is already in receive buffer (must be int)
    int bufferAvailable = 0;
    // data in receive buffer is base64 file data
    bool isBase64File = false;
    // the base64 encoded string downloaded anount
    int downloadByteLen = 0;
    // pad (=) length checking from tail of encoded string of file/blob data
    int base64PadLenTail = 0;
    // pad (=) length checking from base64 encoded string signature (begins with "file,base64, and "blob,base64,)
    // of file/blob data
    int base64PadLenSignature = 0;
    // the tcp client pointer
    Client *client = nullptr;
    // the chunk state info
    fb_esp_chunk_state_info chunkState;

public:
    int available()
    {
        if (client)
            return client->available();
        return false;
    }
};

#ifdef ENABLE_RTDB

struct fb_esp_rtdb_address_t
{
    int dout = 0;
    int din = 0;
    int priority = 0;
    int query = 0;
};

struct fb_esp_rtdb_request_data_info
{
    fb_esp_data_type type = d_any;
    struct fb_esp_rtdb_address_t address;
    int value_subtype = 0;
    MB_String etag;
    size_t blobSize = 0;
};

typedef struct fb_esp_rtdb_upload_status_info_t
{
    size_t progress = 0;
    fb_esp_rtdb_upload_status status = fb_esp_rtdb_upload_status_unknown;
    MB_String localFileName;
    MB_String remotePath;
    int size = 0;
    int elapsedTime = 0;
    MB_String errorMsg;

} RTDB_UploadStatusInfo;

typedef struct fb_esp_rtdb_download_status_info_t
{
    size_t progress = 0;
    fb_esp_rtdb_download_status status = fb_esp_rtdb_download_status_unknown;
    MB_String localFileName;
    MB_String remotePath;
    int size = 0;
    int elapsedTime = 0;
    MB_String errorMsg;

} RTDB_DownloadStatusInfo;

typedef void (*RTDB_UploadProgressCallback)(RTDB_UploadStatusInfo);
typedef void (*RTDB_DownloadProgressCallback)(RTDB_DownloadStatusInfo);

struct fb_esp_rtdb_request_info_t
{
    MB_String path;
    MB_String pre_payload;
    MB_String post_payload;
    MB_String payload;
    MB_String filename;
    fb_esp_request_method method = http_get;
    struct fb_esp_rtdb_request_data_info data;
    fb_esp_rtdb_task_type task_type = fb_esp_rtdb_task_undefined;
    bool queue = false;
    bool async = false;
    size_t fileSize = 0;
#if defined(FIREBASE_ESP_CLIENT)
    fb_esp_mem_storage_type storageType = mem_storage_type_undefined;
#else
    uint8_t storageType = StorageType::UNDEFINED;
#endif
    int progress = -1;
    RTDB_UploadStatusInfo *uploadStatusInfo = nullptr;
    RTDB_DownloadStatusInfo *downloadStatusInfo = nullptr;
    RTDB_UploadProgressCallback uploadCallback = NULL;
    RTDB_DownloadProgressCallback downloadCallback = NULL;
};

#endif

typedef struct fb_esp_spi_ethernet_module_t
{
#if defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)
#ifdef INC_ENC28J60_LWIP
    ENC28J60lwIP *enc28j60 = nullptr;
#endif
#ifdef INC_W5100_LWIP
    Wiznet5100lwIP *w5100 = nullptr;
#endif
#ifdef INC_W5500_LWIP
    Wiznet5500lwIP *w5500 = nullptr;
#endif

#elif defined(MB_ARDUINO_PICO)

#endif

} SPI_ETH_Module;

struct fb_esp_auth_token_info_t
{
    const char *legacy_token = "";
    MB_String auth_type;
    MB_String jwt;
    MB_String scope;
    unsigned long expires = 0;
    /* milliseconds count when last expiry time was set */
    unsigned long last_millis = 0;
    fb_esp_auth_token_type token_type = token_type_undefined;
    fb_esp_auth_token_status status = token_status_uninitialized;
    struct fb_esp_auth_token_error_t error;
};

struct fb_esp_auth_signin_token_t
{
    MB_String uid;
    MB_String claims;
};

struct fb_esp_service_account_data_info_t
{
    MB_String client_email;
    MB_String client_id;
    MB_String project_id;
    MB_String private_key_id;
    const char *private_key = "";
};

struct fb_esp_auth_signin_user_t
{
    MB_String email;
    MB_String password;
};

struct fb_esp_auth_cert_t
{
    const char *data = NULL;
    MB_String file;
#if defined(FIREBASE_ESP_CLIENT)
    fb_esp_mem_storage_type file_storage = mem_storage_type_flash;
#else
    uint8_t file_storage = StorageType::UNDEFINED;
#endif
};

struct fb_esp_service_account_file_info_t
{
    MB_String path;
#if defined(FIREBASE_ESP_CLIENT)
    fb_esp_mem_storage_type storage_type = mem_storage_type_flash;
#else
    uint8_t storage_type = StorageType::UNDEFINED;
#endif
};

struct fb_esp_service_account_t
{
    struct fb_esp_service_account_data_info_t data;
    struct fb_esp_service_account_file_info_t json;
};

struct fb_esp_auth_signin_provider_t
{
    struct fb_esp_auth_signin_user_t user;
    struct fb_esp_auth_signin_token_t token;
};

struct fb_esp_token_signer_resources_t
{
    int step = 0;
    bool test_mode = false;
    bool signup = false;
    bool anonymous = false;
    bool idTokenCustomSet = false;
    bool accessTokenCustomSet = false;
    bool customTokenCustomSet = false;
    bool tokenTaskRunning = false;
    /* last token request milliseconds count */
    unsigned long lastReqMillis = 0;
    unsigned long preRefreshSeconds = DEFAULT_AUTH_TOKEN_PRE_REFRESH_SECONDS;
    unsigned long expiredSeconds = DEFAULT_AUTH_TOKEN_EXPIRED_SECONDS;
    /* request time out period (interval) */
    unsigned long reqTO = DEFAULT_REQUEST_TIMEOUT;
    MB_String customHeaders;
    MB_String pk;
    size_t hashSize = 32; // SHA256 size (256 bits or 32 bytes)
    size_t signatureSize = 256;
#if defined(ESP32)
    uint8_t *hash = nullptr;
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO)
    char *hash = nullptr;
#endif
    unsigned char *signature = nullptr;
    MB_String encHeader;
    MB_String encPayload;
    MB_String encHeadPayload;
    MB_String encSignature;
#if defined(ESP32)
    mbedtls_pk_context *pk_ctx = nullptr;
    mbedtls_entropy_context *entropy_ctx = nullptr;
    mbedtls_ctr_drbg_context *ctr_drbg_ctx = nullptr;
#endif
    struct fb_esp_auth_token_info_t tokens;
    struct fb_esp_auth_token_error_t verificationError;
    struct fb_esp_auth_token_error_t resetPswError;
    struct fb_esp_auth_token_error_t signupError;
    struct fb_esp_auth_token_error_t deleteError;
};

#ifdef ENABLE_RTDB
struct fb_esp_stream_info_t
{
    MB_String stream_path;
    MB_String path;
    MB_String data;
    MB_VECTOR<uint8_t> *blob = nullptr;
    MB_String data_type_str;
    MB_String event_type_str;
    uint8_t data_type = 0;
    int idx = -1;
    /*
    fb_esp_data_type m_type = d_any;
    MB_String m_data;
    MB_String m_path;
    MB_String m_type_str;
    MB_String m_event_type_str;
    */
    FirebaseJson *m_json = nullptr;
    size_t payload_length = 0;
    size_t max_payload_length = 0;
    int httpCode = 0;
};
#endif

struct fb_esp_cfg_int_t
{
    bool fb_multiple_requests = false;
    bool fb_processing = false;
    bool fb_rtoken_requested = false;
    uint8_t fb_stream_idx = 0;

    bool fb_reconnect_wifi = false;
    unsigned long fb_last_reconnect_millis = 0;
    bool net_once_connected = false;
    unsigned long fb_last_jwt_begin_step_millis = 0;
    unsigned long fb_last_jwt_generation_error_cb_millis = 0;
    unsigned long fb_last_request_token_cb_millis = 0;
    unsigned long fb_last_stream_timeout_cb_millis = 0;
    unsigned long fb_last_time_sync_millis = 0;
    unsigned long fb_last_ntp_sync_timeout_millis = 0;
    bool fb_clock_rdy = false;
    /* flag set when NTP time server synching has been started */
    bool fb_clock_synched = false;
    float fb_gmt_offset = 0;
    uint8_t fb_float_digits = 5;
    uint8_t fb_double_digits = 9;
    bool fb_auth_uri = false;
    MB_VECTOR<uint32_t> sessions;
    MB_VECTOR<uint32_t> queueSessions;

    MB_String auth_token;
    MB_String refresh_token;
    MB_String client_id;
    MB_String client_secret;
    uint16_t rtok_len = 0;
    uint16_t atok_len = 0;
    uint16_t ltok_len = 0;
    uint16_t email_crc = 0, password_crc = 0, client_email_crc = 0, project_id_crc = 0, priv_key_crc = 0, uid_crc = 0;

    bool stream_loop_task_enable = true;
    bool deploy_loop_task_enable = true;
#if defined(ESP32) || defined(MB_ARDUINO_PICO)
    TaskHandle_t resumable_upload_task_handle = NULL;
    TaskHandle_t functions_check_task_handle = NULL;
    TaskHandle_t functions_deployment_task_handle = NULL;

    TaskHandle_t stream_task_handle = NULL;
    TaskHandle_t queue_task_handle = NULL;
    size_t stream_task_stack_size = STREAM_TASK_STACK_SIZE;
    uint8_t stream_task_priority = 3;
    uint8_t stream_task_cpu_core = 1;
#if defined(ESP32)
    uint16_t stream_task_delay_ms = 10;
#else
    uint16_t stream_task_delay_ms = 100;
#endif
    size_t queue_task_stack_size = QUEUE_TASK_STACK_SIZE;
    uint8_t queue_task_priority = 1;
    uint8_t queue_task_cpu_core = 1;
#if defined(ESP32)
    uint16_t queue_task_delay_ms = 10;
#else
    uint16_t queue_task_delay_ms = 100;
#endif
#endif
};

struct fb_esp_rtdb_config_t
{
    bool data_type_stricted = false;
    size_t upload_buffer_size = 128;

    // unused, call fbdo.setResponseSize instead
    // size_t download_buffer_size = 256;
};

struct fb_esp_url_info_t
{
    MB_String host;
    MB_String uri;
    MB_String auth;
};

#if defined(FIREBASE_ESP_CLIENT)

#if defined(ENABLE_FIRESTORE)

typedef struct fb_esp_cfs_upload_status_info_t
{
    size_t progress = 0;
    fb_esp_cfs_upload_status status = fb_esp_cfs_upload_status_unknown;
    int size = 0;
    int elapsedTime = 0;
    MB_String errorMsg;

} CFS_UploadStatusInfo;

typedef void (*CFS_UploadProgressCallback)(CFS_UploadStatusInfo);

struct fb_esp_cfs_config_t
{
    CFS_UploadProgressCallback upload_callback = NULL;
};

#endif

#if defined(ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE)
// shared struct between fcs and gcs
struct fb_esp_fcs_file_list_item_t
{
    MB_String name;
    MB_String bucket;
    MB_String contentType;
    size_t size = 0;
    unsigned long generation = 0;
};

#endif

#ifdef ENABLE_GC_STORAGE

struct fb_esp_gcs_config_t
{
    size_t upload_buffer_size = 2048;
    size_t download_buffer_size = 2048;
};

typedef struct fb_esp_gcs_upload_status_info_t
{
    size_t progress = 0;
    fb_esp_gcs_upload_status status = fb_esp_gcs_upload_status_unknown;
    MB_String localFileName;
    MB_String remoteFileName;
    int fileSize = 0;
    int elapsedTime = 0;
    MB_String errorMsg;

} UploadStatusInfo;

typedef struct fb_esp_gcs_download_status_info_t
{
    size_t progress = 0;
    fb_esp_gcs_download_status status = fb_esp_gcs_download_status_unknown;
    MB_String localFileName;
    MB_String remoteFileName;
    int fileSize = 0;
    int elapsedTime = 0;
    MB_String errorMsg;

} DownloadStatusInfo;

typedef void (*UploadProgressCallback)(UploadStatusInfo);
typedef void (*DownloadProgressCallback)(DownloadStatusInfo);

#endif

#ifdef ENABLE_FB_FUNCTIONS

struct fb_esp_functions_config_t
{
    size_t upload_buffer_size = 2048;
    size_t download_buffer_size = 2048;
};

#endif

#if defined(ENABLE_FB_STORAGE)
struct fb_esp_fcs_config_t
{
    size_t upload_buffer_size = 2048;
    size_t download_buffer_size = 2048;
};

typedef struct fb_esp_fcs_upload_status_info_t
{
    size_t progress = 0;
    fb_esp_fcs_upload_status status = fb_esp_fcs_upload_status_unknown;
    MB_String localFileName;
    MB_String remoteFileName;
    int fileSize = 0;
    int elapsedTime = 0;
    MB_String errorMsg;

} FCS_UploadStatusInfo;

typedef struct fb_esp_fcs_download_status_info_t
{
    size_t progress = 0;
    fb_esp_fcs_download_status status = fb_esp_fcs_download_status_unknown;
    MB_String localFileName;
    MB_String remoteFileName;
    int fileSize = 0;
    int elapsedTime = 0;
    MB_String errorMsg;

} FCS_DownloadStatusInfo;

typedef void (*FCS_UploadProgressCallback)(FCS_UploadStatusInfo);
typedef void (*FCS_DownloadProgressCallback)(FCS_DownloadStatusInfo);

#endif

#if defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE)
// shared struct between fcs and gcs
struct fb_esp_fcs_file_list_t
{
    MB_VECTOR<struct fb_esp_fcs_file_list_item_t> items;
};
#endif

#endif

typedef struct token_info_t
{
    fb_esp_auth_token_type type = token_type_undefined;
    fb_esp_auth_token_status status = token_status_uninitialized;
    struct fb_esp_auth_token_error_t error;
} TokenInfo;

typedef void (*TokenStatusCallback)(TokenInfo);

struct fb_esp_client_timeout_t
{
    // WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
    uint16_t wifiReconnect = MIN_WIFI_RECONNECT_TIMEOUT;

    // Socket connection and ssl handshake timeout in ms (1 sec - 1 min).
    unsigned long socketConnection = DEFAULT_SOCKET_CONN_TIMEOUT;

    // unused.
    unsigned long sslHandshake = 0;

    // Server response read timeout in ms (1 sec - 1 min).
    unsigned long serverResponse = DEFAULT_SERVER_RESPONSE_TIMEOUT;

    // RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
    unsigned long rtdbKeepAlive = DEFAULT_RTDB_KEEP_ALIVE_TIMEOUT;

    // RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
    uint16_t rtdbStreamReconnect = MIN_RTDB_STREAM_RECONNECT_INTERVAL;

    // RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
    // will return false (error) when it called repeatedly in loop.
    uint16_t rtdbStreamError = MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL;

    uint16_t tokenGenerationBeginStep = MIN_TOKEN_GENERATION_BEGIN_STEP_INTERVAL;

    uint16_t tokenGenerationError = MIN_TOKEN_GENERATION_ERROR_INTERVAL;

    // NTP server sync timeout in ms
    uint16_t ntpServerRequest = MIN_NTP_SERVER_SYNC_TIME_OUT;
};

struct fb_esp_cfg_t
{
    struct fb_esp_service_account_t service_account;
    // deprecated, use database_url instead
    MB_String host;
    MB_String database_url;
    MB_String api_key;
    float time_zone = 0;
    uint8_t tcp_data_sending_retry = 1;
    size_t async_close_session_max_request = 100;
    struct fb_esp_auth_cert_t cert;
    struct fb_esp_token_signer_resources_t signer;
    struct fb_esp_cfg_int_t internal;
    TokenStatusCallback token_status_callback = NULL;
    // deprecated
    int8_t max_token_generation_retry = 0;
    fb_esp_wifi wifi;
    struct fb_esp_rtdb_config_t rtdb;
#if defined(ENABLE_GC_STORAGE)
    struct fb_esp_gcs_config_t gcs;
#endif
#if defined(ENABLE_FIRESTORE)
    struct fb_esp_cfs_config_t cfs;
#endif
#if defined(ENABLE_FB_STORAGE)
    struct fb_esp_fcs_config_t fcs;
#endif

#if defined(ENABLE_FB_FUNCTIONS)
    struct fb_esp_functions_config_t functions;
#endif

    SPI_ETH_Module spi_ethernet_module;
    struct fb_esp_client_timeout_t timeout;

public:
    fb_esp_cfg_t(){};
    ~fb_esp_cfg_t() { wifi.clearAP(); };
};

#ifdef ENABLE_RTDB
struct fb_esp_rtdb_info_t
{
    bool data_tmo = false;
    bool no_content_req = false;
    bool stream_data_changed = false;
    bool stream_path_changed = false;
    bool data_available = false;
    fb_esp_http_connection_type http_req_conn_type = fb_esp_http_connection_type_undefined;
    fb_esp_http_connection_type http_resp_conn_type = fb_esp_http_connection_type_undefined;
    bool data_mismatch = false;
    bool path_not_found = false;
    bool pause = false;
    bool stream_stop = true;
    bool async = false;
    bool new_stream = false;
    size_t async_count = 0;

    uint8_t connection_status = 0;
    uint32_t queue_ID = 0;
    uint8_t max_retry = 0;
    fb_esp_request_method req_method = http_put;
    fb_esp_data_type req_data_type = fb_esp_data_type::d_any;
    fb_esp_data_type resp_data_type = fb_esp_data_type::d_any;
    uint16_t data_crc = 0;
    MB_String path;
    MB_String raw;
    MB_String stream_path;
    MB_String push_name;
    MB_String redirect_url;
    MB_String event_type;
    MB_String data_type_str;
    MB_String req_etag;
    MB_String resp_etag;
    float priority;
#if defined(FIREBASE_ESP_CLIENT)
    fb_esp_mem_storage_type storage_type = mem_storage_type_flash;
#else
    uint8_t storage_type = StorageType::UNDEFINED;
#endif
    uint8_t redirect_count = 0;

    uint16_t max_blob_size = MAX_BLOB_PAYLOAD_SIZE;

    unsigned long stream_tmo_Millis = 0;
    unsigned long stream_resume_millis = 0;
    unsigned long data_millis = 0;

    MB_VECTOR<uint8_t> *blob = nullptr;
    int isBlobPtr = false;

    bool priority_val_flag = false;
    bool priority_json_flag = false;
    bool shallow_flag = false;
    int read_tmo = -1;
    MB_String write_limit;

    MB_String filename;
    size_t file_size = 0;

    struct fb_esp_stream_info_t stream;

#if defined(ESP32) || defined(MB_ARDUINO_PICO)
    bool stream_loop_task_enable = false;
#endif

    RTDB_UploadStatusInfo cbUploadInfo;
    RTDB_DownloadStatusInfo cbDownloadInfo;
};

#endif

#if defined(FIREBASE_ESP_CLIENT)

#ifdef ENABLE_FB_FUNCTIONS

typedef struct fb_esp_function_operation_info_t
{
    fb_esp_functions_operation_status status = fb_esp_functions_operation_status_unknown;
    MB_String errorMsg;
    MB_String triggerUrl;
    MB_String functionId;
} FunctionsOperationStatusInfo;

typedef void (*FunctionsOperationCallback)(FunctionsOperationStatusInfo);

struct fb_esp_deploy_task_info_t
{
    MB_String projectId;
    MB_String locationId;
    MB_String functionId;
    MB_String policy;
    MB_String httpsTriggerUrl;
    FunctionsConfig *config = nullptr;
    fb_esp_functions_creation_step step = fb_esp_functions_creation_step_idle;
    fb_esp_functions_creation_step nextStep = fb_esp_functions_creation_step_idle;

    FunctionsOperationStatusInfo *statusInfo = nullptr;
    bool active = false;
    bool _delete = false;
    bool done = false;
    bool setPolicy = false;
    bool patch = false;
    FunctionsOperationCallback callback = NULL;
    FirebaseData *fbdo = nullptr;
    MB_String uploadUrl;
};

struct fb_esp_functions_info_t
{
    fb_esp_functions_request_type requestType = fb_esp_functions_request_type_undefined;
    int contentLength = 0;
    fb_esp_functions_operation_status last_status = fb_esp_functions_operation_status_unknown;
    FunctionsOperationStatusInfo cbInfo;
    MB_String payload;
    MB_String filepath;
    fb_esp_mem_storage_type storageType = mem_storage_type_undefined;
    uint32_t fileSize = 0;
};

struct fb_esp_functions_https_trigger_t
{
    MB_String url;
};

struct fb_esp_functions_event_trigger_t
{
    MB_String eventType;
    MB_String resource;
    MB_String service;
    MB_String failurePolicy;
};

struct fb_esp_functions_req_t
{
    MB_String projectId;
    MB_String locationId;
    MB_String functionId;
    MB_String databaseURL;
    MB_String bucketID;
    MB_String payload;
    MB_String filter;
    MB_String host;
    MB_String uri;
    MB_String filePath;
    const uint8_t *pgmArc = nullptr;
    size_t pgmArcLen = 0;
    fb_esp_mem_storage_type storageType = mem_storage_type_undefined;
    MB_String policyVersion;
    size_t versionId = 0;
    size_t pageSize = 0;
    MB_String pageToken;
    MB_VECTOR<MB_String> *updateMask = nullptr;
    fb_esp_functions_request_type requestType = fb_esp_functions_request_type_undefined;
};

#endif

#if defined(ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE)
// shared struct between fcs and gcs
struct fb_esp_gcs_meta_info_t
{
    MB_String name;
    MB_String bucket;
    unsigned long generation = 0;
    unsigned long metageneration = 0;
    MB_String contentType;
    size_t size = 0;
    MB_String etag;
    MB_String crc32;
    MB_String downloadTokens;
    MB_String mediaLink;
};
#endif

#ifdef ENABLE_GC_STORAGE

typedef struct fb_esp_gcs_request_properties_t
{
    MB_String acl; // array
    MB_String cacheControl;
    MB_String contentDisposition;
    MB_String contentEncoding;
    MB_String contentLanguage;
    MB_String contentType;
    MB_String crc32c;
    MB_String customTime;     // date time
    MB_String eventBasedHold; // boolean
    MB_String md5Hash;
    MB_String metadata; // object
    MB_String name;
    MB_String storageClass;
    MB_String temporaryHold; // boolean
} RequestProperties;

typedef struct fb_esp_gcs_get_options_t
{
    MB_String generation;
    MB_String ifGenerationMatch;
    MB_String ifGenerationNotMatch;
    MB_String ifMetagenerationMatch;
    MB_String ifMetagenerationNotMatch;
    MB_String projection;
} StorageGetOptions;

struct fb_esp_gcs_info_t
{
    UploadStatusInfo cbUploadInfo;
    DownloadStatusInfo cbDownloadInfo;
    fb_esp_gcs_request_type requestType = fb_esp_gcs_request_type_undefined;
    fb_esp_mem_storage_type storage_type = mem_storage_type_undefined;
    int contentLength = 0;
    MB_String payload;
    struct fb_esp_gcs_meta_info_t meta;
};

typedef struct fb_esp_gcs_upload_options_t
{
    MB_String contentEncoding;
    MB_String ifGenerationMatch;        // long
    MB_String ifGenerationNotMatch;     // long
    MB_String ifMetagenerationMatch;    // long
    MB_String ifMetagenerationNotMatch; // long
    MB_String kmsKeyName;
    MB_String projection;
    MB_String predefinedAcl;
} UploadOptions;

typedef struct fb_esp_gcs_delete_options_t
{
    MB_String generation;
    MB_String ifGenerationMatch;        // long
    MB_String ifGenerationNotMatch;     // long
    MB_String ifMetagenerationMatch;    // long
    MB_String ifMetagenerationNotMatch; // long
} DeleteOptions;

typedef struct fb_esp_gcs_list_options_t
{
    MB_String delimiter;
    MB_String endOffset;
    MB_String includeTrailingDelimiter; // bool
    MB_String maxResults;               // number
    MB_String pageToken;
    MB_String prefix;
    MB_String projection;
    MB_String startOffset;
    MB_String versions; // bool
} ListOptions;

struct fb_esp_gcs_req_t
{
    MB_String remoteFileName;
    MB_String localFileName;
    MB_String bucketID;
    MB_String mime;
    MB_String location;
    int chunkIndex = -1;
    int chunkRange = -1;
    int chunkPos = 0;
    int chunkLen = 0;
    size_t fileSize = 0;
    int progress = -1;
    ListOptions *listOptions = nullptr;
    StorageGetOptions *getOptions = nullptr;
    UploadOptions *uploadOptions = nullptr;
    DeleteOptions *deleteOptions = nullptr;
    RequestProperties *requestProps = nullptr;
    fb_esp_mem_storage_type storageType = mem_storage_type_undefined;
    fb_esp_gcs_request_type requestType = fb_esp_gcs_request_type_undefined;
    UploadStatusInfo *uploadStatusInfo = nullptr;
    DownloadStatusInfo *downloadStatusInfo = nullptr;
    UploadProgressCallback uploadCallback = NULL;
    DownloadProgressCallback downloadCallback = NULL;
};

struct fb_gcs_upload_resumable_task_info_t
{
    FirebaseData *fbdo = nullptr;
    struct fb_esp_gcs_req_t req;
    bool done = false;
};

#endif

#ifdef ENABLE_FCM
struct fb_esp_fcm_legacy_notification_payload_t
{
    MB_String title;              // string all
    MB_String body;               // string all
    MB_String icon;               // string Andoid, web
    MB_String click_action;       // string all
    MB_String sound;              // string iOS, Android
    MB_String badge;              // number iOS
    MB_String subtitle;           // string iOS
    MB_String body_loc_key;       // string iOS, Android
    MB_String body_loc_args;      // array of string [] iOS, Android
    MB_String title_loc_key;      // string iOS, Android
    MB_String title_loc_args;     // array of string [] iOS, Android
    MB_String android_channel_id; // string Android
    MB_String tag;                // string Android
    MB_String color;              // string Android
};

struct fb_esp_fcm_legacy_http_message_option_t
{
    MB_String priority;                // string
    MB_String collapse_key;            // string
    MB_String time_to_live;            // number
    MB_String restricted_package_name; // string
    MB_String mutable_content;         // boolean
    MB_String content_available;       // boolean
    MB_String dry_run;                 // boolean
    MB_String direct_boot_ok;          // boolean
};

struct fb_esp_fcm_legacy_http_message_payload_t
{
    struct fb_esp_fcm_legacy_notification_payload_t notification;
    MB_String data;
};

struct fb_esp_fcm_legacy_http_message_target_t
{
    MB_String to;               // string
    MB_String registration_ids; // array of string []
    MB_String condition;        // string
};

struct fb_esp_fcm_legacy_http_message_info_t
{
    struct fb_esp_fcm_legacy_http_message_target_t targets;
    struct fb_esp_fcm_legacy_http_message_option_t options;
    struct fb_esp_fcm_legacy_http_message_payload_t payloads;
};

struct fb_esp_fcm_http_v1_notification_t
{
    MB_String title; // string
    MB_String body;  // string
    MB_String image; // string
};

struct fb_esp_fcm_http_v1_fcm_options_t
{
    MB_String analytics_label; // string, Label associated with the message's analytics data.
};

struct fb_esp_fcm_http_v1_android_fcm_options_t
{
    MB_String analytics_label; // string, Label associated with the message's analytics data.
};

struct fb_esp_fcm_http_v1_android_light_settings_color_t
{
    MB_String red;   // string
    MB_String green; // string
    MB_String blue;  // string
    MB_String alpha; // string
};

struct fb_esp_fcm_http_v1_android_light_settings_t
{
    struct fb_esp_fcm_http_v1_android_light_settings_color_t color; // object {}
    MB_String light_on_duration;                                    // string
    MB_String light_off_duration;                                   // string
};

struct fb_esp_fcm_http_v1_android_noti_t
{
    MB_String title;                   // string
    MB_String body;                    // string
    MB_String icon;                    // string
    MB_String color;                   // string
    MB_String sound;                   // string
    MB_String tag;                     // string
    MB_String click_action;            // string
    MB_String body_loc_key;            // string
    MB_String body_loc_args;           // array of string []
    MB_String title_loc_key;           // string
    MB_String title_loc_args;          // array of string []
    MB_String channel_id;              // string
    MB_String ticker;                  // string
    MB_String sticky;                  // boolean
    MB_String event_time;              // string
    MB_String local_only;              // boolean
    MB_String notification_priority;   // enum
    MB_String default_sound;           // boolean
    MB_String default_vibrate_timings; // boolean
    MB_String default_light_settings;  // boolean
    MB_String vibrate_timings;         // array of string []
    MB_String visibility;              // enum
    MB_String notification_count;      // integer
    struct fb_esp_fcm_http_v1_android_light_settings_t light_settings;
    MB_String image; // string
};

struct fb_esp_fcm_http_v1_android_config_t
{
    MB_String collapse_key;            // string
    MB_String priority;                // enum
    MB_String ttl;                     // string
    MB_String restricted_package_name; // string
    MB_String data;                    /// object {} Arbitrary key/value payload
    fb_esp_fcm_http_v1_android_noti_t notification;
    struct fb_esp_fcm_http_v1_android_fcm_options_t fcm_options;
    MB_String direct_boot_ok; // boolean
};

struct fb_esp_fcm_http_v1_apns_fcm_opt_t
{
    MB_String analytics_label; // string Label associated with the message's analytics data
    MB_String image;           // string contains the URL of an image that is going to be displayed in a notification.
};

struct fb_esp_fcm_http_v1_webpush_fcm_opt_t
{
    MB_String link;            // string, The link to open when the user clicks on the notification.
    MB_String analytics_label; // string, Label associated with the message's analytics data.
};

struct fb_esp_fcm_http_v1_apns_config_t
{
    MB_String headers;      // object {} http header key/value defined in Apple Push Notification Service
    MB_String payload;      // object {} APNs payload as a JSON object
    MB_String notification; // object {}
    struct fb_esp_fcm_http_v1_apns_fcm_opt_t fcm_options;
};

struct fb_esp_fcm_http_v1_webpush_config_t
{
    MB_String headers;      // object {} http header key/value defined in webpush protocol.
    MB_String data;         // object {} abitrary key/value payload
    MB_String notification; // object {} Web Notification options as a JSON object
    struct fb_esp_fcm_http_v1_webpush_fcm_opt_t fcm_options;
};

struct fb_esp_fcm_http_v1_message_info_t
{
    MB_String token;     // string
    MB_String topic;     // string
    MB_String condition; // string
    struct fb_esp_fcm_http_v1_fcm_options_t fcm_options;
    struct fb_esp_fcm_http_v1_notification_t notification;
    MB_String data; // object {} abitrary key/value payload
    struct fb_esp_fcm_http_v1_android_config_t android;
    struct fb_esp_fcm_http_v1_apns_config_t apns;
    struct fb_esp_fcm_http_v1_webpush_config_t webpush;
};

struct fb_esp_fcm_info_t
{
    MB_String payload;
};

#endif

#if defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE)
struct fb_esp_fcs_info_t
{
#ifdef ENABLE_FB_STORAGE
    fb_esp_fcs_request_type requestType = fb_esp_fcs_request_type_undefined;
    size_t fileSize = 0;
    fb_esp_mem_storage_type storage_type = mem_storage_type_undefined;
    int contentLength = 0;

    FCS_UploadStatusInfo cbUploadInfo;
    FCS_DownloadStatusInfo cbDownloadInfo;
#endif
    struct fb_esp_gcs_meta_info_t meta;
    struct fb_esp_fcs_file_list_t files;
};
#endif

#ifdef ENABLE_FB_STORAGE

struct fb_esp_fcs_req_t
{
    MB_String remoteFileName;
    MB_String localFileName;
    MB_String bucketID;
    MB_String mime;
    const uint8_t *pgmArc = nullptr;
    size_t pgmArcLen = 0;
    size_t fileSize = 0;
    int progress = -1;
    fb_esp_mem_storage_type storageType = mem_storage_type_undefined;
    fb_esp_fcs_request_type requestType = fb_esp_fcs_request_type_undefined;
    FCS_UploadStatusInfo *uploadStatusInfo = nullptr;
    FCS_DownloadStatusInfo *downloadStatusInfo = nullptr;
    FCS_UploadProgressCallback uploadCallback = NULL;
    FCS_DownloadProgressCallback downloadCallback = NULL;
};

#endif

#ifdef ENABLE_FIRESTORE
struct fb_esp_firestore_info_t
{
    fb_esp_firestore_request_type requestType = fb_esp_firestore_request_type_undefined;
    CFS_UploadStatusInfo cbUploadInfo;
    int contentLength = 0;
    MB_String payload;
    bool async = false;
};

struct fb_esp_firestore_transaction_read_only_option_t
{
    MB_String readTime;
};

struct fb_esp_firestore_transaction_read_write_option_t
{
    MB_String retryTransaction;
};

typedef struct fb_esp_firestore_transaction_options_t
{
    fb_esp_firestore_transaction_read_only_option_t readOnly;
    fb_esp_firestore_transaction_read_write_option_t readWrite;
} TransactionOptions;

struct fb_esp_firestore_req_t
{
    MB_String projectId;
    MB_String databaseId;
    MB_String collectionId;
    MB_String documentId;
    MB_String documentPath;
    MB_String mask;
    MB_String updateMask;
    MB_String payload;
    MB_String exists;
    MB_String updateTime;
    MB_String readTime;
    MB_String transaction;
    int pageSize = 10;
    MB_String pageToken;
    MB_String orderBy;
    bool showMissing = false;
    bool async = false;
    size_t size = 0;
    fb_esp_firestore_request_type requestType = fb_esp_firestore_request_type_undefined;
    CFS_UploadStatusInfo *uploadStatusInfo = nullptr;
    CFS_UploadProgressCallback uploadCallback = NULL;
    FB_ResponseCallback responseCallback = NULL;
    int progress = -1;
    unsigned long requestTime = 0;
};

struct fb_esp_firestore_document_write_field_transforms_t
{
    MB_String fieldPath; // string The path of the field. See Document.fields for the field path syntax reference.
    fb_esp_firestore_transform_type transform_type = fb_esp_firestore_transform_type_undefined;
    MB_String transform_content; // string of enum of ServerValue for setToServerValue, string of object of values for increment, maximum and minimum
    //, string of array object for appendMissingElements or removeAllFromArray.
};

struct fb_esp_firestore_document_write_document_transform_t
{
    MB_String transform_document_path;                                                     // The relative path of document to transform.
    MB_VECTOR<struct fb_esp_firestore_document_write_field_transforms_t> field_transforms; // array of fb_esp_firestore_document_write_field_transforms_t data.
};

struct fb_esp_firestore_document_precondition_t
{
    MB_String exists;      // bool
    MB_String update_time; // string of timestamp. When set, the target document must exist and have been last updated at that time.
    // A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits.Examples : "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".
};

struct fb_esp_firestore_document_write_t
{
    MB_String update_masks; // string The fields to update. Use comma (,) to separate between the field names
    struct fb_esp_firestore_document_write_field_transforms_t update_transforms;
    struct fb_esp_firestore_document_precondition_t current_document; // An optional precondition on the document.
    fb_esp_firestore_document_write_type type = fb_esp_firestore_document_write_type_undefined;
    MB_String update_document_content;                                              // A document object to write for fb_esp_firestore_document_write_type_update.
    MB_String update_document_path;                                                 // The relative path of document to update for fb_esp_firestore_document_write_type_update.
    MB_String delete_document_path;                                                 // The relative path of document to delete for fb_esp_firestore_document_write_type_delete.
    struct fb_esp_firestore_document_write_document_transform_t document_transform; // for fb_esp_firestore_document_write_type_transform
};

#endif

#endif

struct fb_esp_session_info_t
{
    int long_running_task = 0;
    FirebaseJson *jsonPtr = nullptr;
    FirebaseJsonArray *arrPtr = nullptr;
    FirebaseJsonData *dataPtr = nullptr;
    int jsonAddr = 0;
    int arrAddr = 0;
    fb_esp_con_mode con_mode = fb_esp_con_mode_undefined;
    bool streaming = false;
    bool buffer_ovf = false;
    bool chunked_encoding = false;
    bool connected = false;
    bool classic_request = false;
    MB_String host;
    unsigned long last_conn_ms = 0;
    int cert_ptr = 0;
    bool cert_updated = false;
    uint32_t conn_timeout = DEFAULT_TCP_CONNECTION_TIMEOUT;

    uint16_t resp_size = 2048;
    fb_esp_response_t response;
    int http_code = 0;
    int content_length = 0;
    size_t payload_length = 0;
    size_t max_payload_length = 0;
    MB_String error;
    int errCode = 0;
#ifdef ENABLE_RTDB
    struct fb_esp_rtdb_info_t rtdb;
#endif
#if defined(FIREBASE_ESP_CLIENT)

#if defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE)
    struct fb_esp_fcs_info_t fcs;
#endif
#ifdef ENABLE_GC_STORAGE
    struct fb_esp_gcs_info_t gcs;
#endif
#ifdef ENABLE_FCM
    struct fb_esp_fcm_info_t fcm;
#endif
#ifdef ENABLE_FIRESTORE
    struct fb_esp_firestore_info_t cfs;
#endif
#ifdef ENABLE_FB_FUNCTIONS
    struct fb_esp_functions_info_t cfn;
#endif
#endif

#if defined(ESP8266)
    uint16_t bssl_rx_size = 512;
    uint16_t bssl_tx_size = 512;
#elif defined(MB_ARDUINO_PICO)
    uint16_t bssl_rx_size = 16384;
    uint16_t bssl_tx_size = 2048;
#endif
};

#if defined(FIREBASE_ESP_CLIENT)

#ifdef ENABLE_FCM
typedef struct fb_esp_fcm_legacy_http_message_info_t FCM_Legacy_HTTP_Message;
typedef struct fb_esp_fcm_http_v1_message_info_t FCM_HTTPv1_JSON_Message;
#endif

#ifdef ENABLE_FB_STORAGE
typedef struct fb_esp_fcs_file_list_t FileList;
#endif

#if defined(ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE)
// shared struct between fcs and gcs
typedef struct fb_esp_gcs_meta_info_t FileMetaInfo;
typedef struct fb_esp_fcs_file_list_item_t FileItem;
#endif

#endif

typedef struct fb_esp_auth_signin_provider_t FirebaseAuth;
typedef struct fb_esp_cfg_t FirebaseConfig;

#if !defined(__AVR__)
typedef std::function<void(void)> callback_function_t;
#endif

//////////////////////////////////////////////
// General use string
static const char fb_esp_pgm_str_1[] PROGMEM = "/";
static const char fb_esp_pgm_str_2[] PROGMEM = ":";
static const char fb_esp_pgm_str_3[] PROGMEM = ",";
static const char fb_esp_pgm_str_4[] PROGMEM = "\"";
static const char fb_esp_pgm_str_5[] PROGMEM = ".";
static const char fb_esp_pgm_str_6[] PROGMEM = "[";
static const char fb_esp_pgm_str_7[] PROGMEM = "?";
static const char fb_esp_pgm_str_8[] PROGMEM = "&";
static const char fb_esp_pgm_str_9[] PROGMEM = " ";
static const char fb_esp_pgm_str_10[] PROGMEM = "{";
static const char fb_esp_pgm_str_11[] PROGMEM = "}";
static const char fb_esp_pgm_str_12[] PROGMEM = "\n";
static const char fb_esp_pgm_str_13[] PROGMEM = "=";
static const char fb_esp_pgm_str_14[] PROGMEM = "-";
static const char fb_esp_pgm_str_15[] PROGMEM = "keep-alive";
static const char fb_esp_pgm_str_16[] PROGMEM = "put";
static const char fb_esp_pgm_str_17[] PROGMEM = "patch";
static const char fb_esp_pgm_str_18[] PROGMEM = "token";
static const char fb_esp_pgm_str_19[] PROGMEM = "false";
static const char fb_esp_pgm_str_20[] PROGMEM = "true";
static const char fb_esp_pgm_str_21[] PROGMEM = "gs://";
static const char fb_esp_pgm_str_22[] PROGMEM = "https://";
static const char fb_esp_pgm_str_23[] PROGMEM = "/v1/projects/";
static const char fb_esp_pgm_str_24[] PROGMEM = "/v1beta1/projects/";
static const char fb_esp_pgm_str_25[] PROGMEM = "https://%[^/]/%s";
static const char fb_esp_pgm_str_26[] PROGMEM = "http://%[^/]/%s";
static const char fb_esp_pgm_str_27[] PROGMEM = "%[^/]/%s";
static const char fb_esp_pgm_str_28[] PROGMEM = "%[^?]?%s";
static const char fb_esp_pgm_str_29[] PROGMEM = "%[^&]";
static const char fb_esp_pgm_str_30[] PROGMEM = "\r\n";
static const char fb_esp_pgm_str_31[] PROGMEM = "googleapis.com";
static const char fb_esp_pgm_str_32[] PROGMEM = "Host: ";
static const char fb_esp_pgm_str_33[] PROGMEM = "Content-Type: ";
static const char fb_esp_pgm_str_34[] PROGMEM = "Content-Length: ";
static const char fb_esp_pgm_str_35[] PROGMEM = "User-Agent: ESP\r\n";
static const char fb_esp_pgm_str_36[] PROGMEM = "Connection: keep-alive\r\n";
static const char fb_esp_pgm_str_37[] PROGMEM = "Connection: close\r\n";
static const char fb_esp_pgm_str_38[] PROGMEM = "PATCH";
static const char fb_esp_pgm_str_39[] PROGMEM = "PUT";
static const char fb_esp_pgm_str_40[] PROGMEM = "POST";
static const char fb_esp_pgm_str_41[] PROGMEM = "GET";
static const char fb_esp_pgm_str_42[] PROGMEM = "DELETE";
static const char fb_esp_pgm_str_43[] PROGMEM = " HTTP/1.1\r\n";
static const char fb_esp_pgm_str_44[] PROGMEM = "Authorization: ";
static const char fb_esp_pgm_str_45[] PROGMEM = "Bearer ";
static const char fb_esp_pgm_str_46[] PROGMEM = "Firebase ";
static const char fb_esp_pgm_str_47[] PROGMEM = "key=";
static const char fb_esp_pgm_str_48[] PROGMEM = "Connection: ";
static const char fb_esp_pgm_str_49[] PROGMEM = "ETag: ";
static const char fb_esp_pgm_str_50[] PROGMEM = "Transfer-Encoding: ";
static const char fb_esp_pgm_str_51[] PROGMEM = "chunked";
static const char fb_esp_pgm_str_52[] PROGMEM = "Location: ";
static const char fb_esp_pgm_str_53[] PROGMEM = "HTTP/1.1 ";
static const char fb_esp_pgm_str_54[] PROGMEM = "\"path\":\"";
static const char fb_esp_pgm_str_55[] PROGMEM = "\"data\":";
static const char fb_esp_pgm_str_56[] PROGMEM = "{\"name\":\"";
static const char fb_esp_pgm_str_57[] PROGMEM = "\"error\" : ";
static const char fb_esp_pgm_str_58[] PROGMEM = "error";
static const char fb_esp_pgm_str_59[] PROGMEM = "null";
static const char fb_esp_pgm_str_60[] PROGMEM = "msg";
static const char fb_esp_pgm_str_61[] PROGMEM = "www";
static const char fb_esp_pgm_str_62[] PROGMEM = "application/json";
static const char fb_esp_pgm_str_63[] PROGMEM = "pageSize";
static const char fb_esp_pgm_str_64[] PROGMEM = "labels";
static const char fb_esp_pgm_str_65[] PROGMEM = "pageToken";
static const char fb_esp_pgm_str_66[] PROGMEM = "name";
static const char fb_esp_pgm_str_67[] PROGMEM = "data";
static const char fb_esp_pgm_str_68[] PROGMEM = "update";
static const char fb_esp_pgm_str_69[] PROGMEM = "delete";
static const char fb_esp_pgm_str_70[] PROGMEM = "updateMask";

// Legacy FCM string
#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
static const char esp_fb_legacy_fcm_pgm_str_1[] PROGMEM = "server_key";
static const char esp_fb_legacy_fcm_pgm_str_2[] PROGMEM = "topic";
static const char esp_fb_legacy_fcm_pgm_str_3[] PROGMEM = "title";
static const char esp_fb_legacy_fcm_pgm_str_4[] PROGMEM = "body";
static const char esp_fb_legacy_fcm_pgm_str_5[] PROGMEM = "icon";
static const char esp_fb_legacy_fcm_pgm_str_6[] PROGMEM = "click_action";
static const char esp_fb_legacy_fcm_pgm_str_7[] PROGMEM = "data";
static const char esp_fb_legacy_fcm_pgm_str_8[] PROGMEM = "priority";
static const char esp_fb_legacy_fcm_pgm_str_9[] PROGMEM = "collapse_key";
static const char esp_fb_legacy_fcm_pgm_str_10[] PROGMEM = "time_to_live";
static const char esp_fb_legacy_fcm_pgm_str_11[] PROGMEM = "/topics/";
static const char esp_fb_legacy_fcm_pgm_str_12[] PROGMEM = "fcm";
static const char esp_fb_legacy_fcm_pgm_str_13[] PROGMEM = "/fcm/send";
static const char esp_fb_legacy_fcm_pgm_str_14[] PROGMEM = "to";
static const char esp_fb_legacy_fcm_pgm_str_15[] PROGMEM = "registration_ids";
static const char esp_fb_legacy_fcm_pgm_str_16[] PROGMEM = "msg";
#endif

// Signer class string
static const char fb_esp_signer_pgm_str_1[] PROGMEM = "type";
static const char fb_esp_signer_pgm_str_2[] PROGMEM = "service_account";
static const char fb_esp_signer_pgm_str_3[] PROGMEM = "project_id";
static const char fb_esp_signer_pgm_str_4[] PROGMEM = ".";
static const char fb_esp_signer_pgm_str_5[] PROGMEM = "private_key_id";
static const char fb_esp_signer_pgm_str_6[] PROGMEM = "private_key";
static const char fb_esp_signer_pgm_str_7[] PROGMEM = "client_email";
static const char fb_esp_signer_pgm_str_8[] PROGMEM = "client_id";
#if !defined(USE_LEGACY_TOKEN_ONLY)
static const char fb_esp_signer_pgm_str_9[] PROGMEM = "securetoken";
static const char fb_esp_signer_pgm_str_10[] PROGMEM = "/v1/token?key=";
static const char fb_esp_signer_pgm_str_11[] PROGMEM = "grantType";
static const char fb_esp_signer_pgm_str_12[] PROGMEM = "refresh_token";
static const char fb_esp_signer_pgm_str_13[] PROGMEM = "refreshToken";
static const char fb_esp_signer_pgm_str_14[] PROGMEM = "id_token";
static const char fb_esp_signer_pgm_str_15[] PROGMEM = "expires_in";
static const char fb_esp_signer_pgm_str_16[] PROGMEM = "user_id";
static const char fb_esp_signer_pgm_str_17[] PROGMEM = "alg";
static const char fb_esp_signer_pgm_str_18[] PROGMEM = "typ";
static const char fb_esp_signer_pgm_str_19[] PROGMEM = "iam";
static const char fb_esp_signer_pgm_str_20[] PROGMEM = "scope";
static const char fb_esp_signer_pgm_str_21[] PROGMEM = "uid";
static const char fb_esp_signer_pgm_str_22[] PROGMEM = "claims";
static const char fb_esp_signer_pgm_str_23[] PROGMEM = "identitytoolkit";
static const char fb_esp_signer_pgm_str_24[] PROGMEM = "email";
static const char fb_esp_signer_pgm_str_25[] PROGMEM = "password";
static const char fb_esp_signer_pgm_str_26[] PROGMEM = "returnSecureToken";
static const char fb_esp_signer_pgm_str_27[] PROGMEM = "/v1/accounts:signUp?key=";
static const char fb_esp_signer_pgm_str_28[] PROGMEM = "JWT";
static const char fb_esp_signer_pgm_str_29[] PROGMEM = "RS256";
static const char fb_esp_signer_pgm_str_30[] PROGMEM = "iss";
static const char fb_esp_signer_pgm_str_31[] PROGMEM = "sub";
static const char fb_esp_signer_pgm_str_32[] PROGMEM = "aud";
static const char fb_esp_signer_pgm_str_33[] PROGMEM = "exp";
static const char fb_esp_signer_pgm_str_34[] PROGMEM = "/google.identity.identitytoolkit.v1.IdentityToolkit";
static const char fb_esp_signer_pgm_str_35[] PROGMEM = "oauth2";
static const char fb_esp_signer_pgm_str_36[] PROGMEM = "iat";
static const char fb_esp_signer_pgm_str_37[] PROGMEM = "auth";
static const char fb_esp_signer_pgm_str_38[] PROGMEM = "devstorage.full_control";
static const char fb_esp_signer_pgm_str_39[] PROGMEM = "datastore";
static const char fb_esp_signer_pgm_str_40[] PROGMEM = "userinfo.email";
static const char fb_esp_signer_pgm_str_41[] PROGMEM = "firebase.database";
static const char fb_esp_signer_pgm_str_42[] PROGMEM = "cloud-platform";
static const char fb_esp_signer_pgm_str_43[] PROGMEM = "/identitytoolkit/v3/relyingparty/";
static const char fb_esp_signer_pgm_str_44[] PROGMEM = "verifyPassword?key=";
static const char fb_esp_signer_pgm_str_45[] PROGMEM = "idToken";
static const char fb_esp_signer_pgm_str_46[] PROGMEM = "refreshToken";
static const char fb_esp_signer_pgm_str_47[] PROGMEM = "expiresIn";
static const char fb_esp_signer_pgm_str_48[] PROGMEM = "localId";
static const char fb_esp_signer_pgm_str_49[] PROGMEM = "/v1/accounts:delete?key=";
static const char fb_esp_signer_pgm_str_50[] PROGMEM = "verifyCustomToken?key=";
static const char fb_esp_signer_pgm_str_51[] PROGMEM = "client_secret";
static const char fb_esp_signer_pgm_str_52[] PROGMEM = "grant_type";
static const char fb_esp_signer_pgm_str_53[] PROGMEM = "urn:ietf:params:oauth:grant-type:jwt-bearer";
static const char fb_esp_signer_pgm_str_54[] PROGMEM = "assertion";
static const char fb_esp_signer_pgm_str_55[] PROGMEM = "error";
static const char fb_esp_signer_pgm_str_56[] PROGMEM = "error_description";
static const char fb_esp_signer_pgm_str_57[] PROGMEM = "access_token";
static const char fb_esp_signer_pgm_str_58[] PROGMEM = "token_type";
static const char fb_esp_signer_pgm_str_59[] PROGMEM = "requestType";
static const char fb_esp_signer_pgm_str_60[] PROGMEM = "VERIFY_EMAIL";
static const char fb_esp_signer_pgm_str_61[] PROGMEM = "getOobConfirmationCode?key=";
static const char fb_esp_signer_pgm_str_62[] PROGMEM = "PASSWORD_RESET";
#endif

// RTDB class string
#if defined(ENABLE_RTDB)
static const char fb_esp_rtdb_pgm_str_1[] PROGMEM = "/.settings/rules";
static const char fb_esp_rtdb_pgm_str_2[] PROGMEM = ".priority";
static const char fb_esp_rtdb_pgm_str_3[] PROGMEM = "rules";
static const char fb_esp_rtdb_pgm_str_4[] PROGMEM = "/.indexOn";
static const char fb_esp_rtdb_pgm_str_5[] PROGMEM = ".read";
static const char fb_esp_rtdb_pgm_str_6[] PROGMEM = ".write";
static const char fb_esp_rtdb_pgm_str_7[] PROGMEM = "\"blob,base64,";
static const char fb_esp_rtdb_pgm_str_8[] PROGMEM = "\"file,base64,";
static const char fb_esp_rtdb_pgm_str_9[] PROGMEM = "text/event-stream";
static const char fb_esp_rtdb_pgm_str_10[] PROGMEM = "/fb_bin_0.tmp";
static const char fb_esp_rtdb_pgm_str_11[] PROGMEM = "null_etag";
static const char fb_esp_rtdb_pgm_str_12[] PROGMEM = "event: ";
static const char fb_esp_rtdb_pgm_str_13[] PROGMEM = "data: ";
static const char fb_esp_rtdb_pgm_str_14[] PROGMEM = "cancel";
static const char fb_esp_rtdb_pgm_str_15[] PROGMEM = "auth_revoked";
static const char fb_esp_rtdb_pgm_str_16[] PROGMEM = "{\"status\":\"ok\"}";
static const char fb_esp_rtdb_pgm_str_17[] PROGMEM = "\".sv\"";
static const char fb_esp_rtdb_pgm_str_18[] PROGMEM = ".json";
static const char fb_esp_rtdb_pgm_str_19[] PROGMEM = "auth=";
static const char fb_esp_rtdb_pgm_str_20[] PROGMEM = "timeout=";
static const char fb_esp_rtdb_pgm_str_21[] PROGMEM = "ms";
static const char fb_esp_rtdb_pgm_str_22[] PROGMEM = "writeSizeLimit=";
static const char fb_esp_rtdb_pgm_str_23[] PROGMEM = "shallow=true";
static const char fb_esp_rtdb_pgm_str_24[] PROGMEM = "orderBy=";
static const char fb_esp_rtdb_pgm_str_25[] PROGMEM = "&limitToFirst=";
static const char fb_esp_rtdb_pgm_str_26[] PROGMEM = "&limitToLast=";
static const char fb_esp_rtdb_pgm_str_27[] PROGMEM = "&startAt=";
static const char fb_esp_rtdb_pgm_str_28[] PROGMEM = "download=";
static const char fb_esp_rtdb_pgm_str_29[] PROGMEM = "print=silent";
static const char fb_esp_rtdb_pgm_str_30[] PROGMEM = "&endAt=";
static const char fb_esp_rtdb_pgm_str_31[] PROGMEM = "&equalTo=";
static const char fb_esp_rtdb_pgm_str_32[] PROGMEM = "format=export";
static const char fb_esp_rtdb_pgm_str_33[] PROGMEM = "X-Firebase-ETag: true\r\n";
static const char fb_esp_rtdb_pgm_str_34[] PROGMEM = "if-match: ";
static const char fb_esp_rtdb_pgm_str_35[] PROGMEM = "Accept: text/event-stream\r\n";
static const char fb_esp_rtdb_pgm_str_36[] PROGMEM = "X-HTTP-Method-Override: ";
static const char fb_esp_rtdb_pgm_str_37[] PROGMEM = "Keep-Alive: timeout=30, max=100\r\n";
static const char fb_esp_rtdb_pgm_str_38[] PROGMEM = "Accept-Encoding: identity;q=1,chunked;q=0.1,*;q=0\r\n";
static const char fb_esp_rtdb_pgm_str_39[] PROGMEM = "{\".sv\": \"timestamp\"}";
static const char fb_esp_rtdb_pgm_str_40[] PROGMEM = "object";
#endif

// FCM class string
#if defined(ENABLE_FCM)
#if defined(FIREBASE_ESP_CLIENT)
static const char fb_esp_fcm_pgm_str_1[] PROGMEM = "fcm";
static const char fb_esp_fcm_pgm_str_2[] PROGMEM = "iid";
static const char fb_esp_fcm_pgm_str_3[] PROGMEM = "/fcm/send";
static const char fb_esp_fcm_pgm_str_4[] PROGMEM = "/messages:send";
static const char fb_esp_fcm_pgm_str_5[] PROGMEM = "/iid/v1";
static const char fb_esp_fcm_pgm_str_6[] PROGMEM = ":batchAdd";
static const char fb_esp_fcm_pgm_str_7[] PROGMEM = ":batchRemove";
static const char fb_esp_fcm_pgm_str_8[] PROGMEM = "/iid/info/";
static const char fb_esp_fcm_pgm_str_9[] PROGMEM = "?details=true";
static const char fb_esp_fcm_pgm_str_10[] PROGMEM = "to";
static const char fb_esp_fcm_pgm_str_11[] PROGMEM = "registration_ids";
static const char fb_esp_fcm_pgm_str_12[] PROGMEM = "priority";
static const char fb_esp_fcm_pgm_str_13[] PROGMEM = "time_to_live";
static const char fb_esp_fcm_pgm_str_14[] PROGMEM = "collapse_key";
static const char fb_esp_fcm_pgm_str_15[] PROGMEM = ":batchImport";
static const char fb_esp_fcm_pgm_str_16[] PROGMEM = "condition";
static const char fb_esp_fcm_pgm_str_17[] PROGMEM = "content_available";
static const char fb_esp_fcm_pgm_str_18[] PROGMEM = "mutable_content";
static const char fb_esp_fcm_pgm_str_19[] PROGMEM = "restricted_package_name";
static const char fb_esp_fcm_pgm_str_20[] PROGMEM = "dry_run";
static const char fb_esp_fcm_pgm_str_21[] PROGMEM = "direct_boot_ok";
static const char fb_esp_fcm_pgm_str_22[] PROGMEM = "title";
static const char fb_esp_fcm_pgm_str_23[] PROGMEM = "body";
static const char fb_esp_fcm_pgm_str_24[] PROGMEM = "sound";
static const char fb_esp_fcm_pgm_str_25[] PROGMEM = "badge";
static const char fb_esp_fcm_pgm_str_26[] PROGMEM = "subtitle";
static const char fb_esp_fcm_pgm_str_27[] PROGMEM = "body_loc_key";
static const char fb_esp_fcm_pgm_str_28[] PROGMEM = "body_loc_args";
static const char fb_esp_fcm_pgm_str_29[] PROGMEM = "title_loc_key";
static const char fb_esp_fcm_pgm_str_30[] PROGMEM = "title_loc_args";
static const char fb_esp_fcm_pgm_str_31[] PROGMEM = "android_channel_id";
static const char fb_esp_fcm_pgm_str_32[] PROGMEM = "icon";
static const char fb_esp_fcm_pgm_str_33[] PROGMEM = "click_action";
static const char fb_esp_fcm_pgm_str_34[] PROGMEM = "tag";
static const char fb_esp_fcm_pgm_str_35[] PROGMEM = "color";
static const char fb_esp_fcm_pgm_str_36[] PROGMEM = "/topics/";
static const char fb_esp_fcm_pgm_str_37[] PROGMEM = "registration_tokens";
static const char fb_esp_fcm_pgm_str_38[] PROGMEM = "application";
static const char fb_esp_fcm_pgm_str_39[] PROGMEM = "sandbox";
static const char fb_esp_fcm_pgm_str_40[] PROGMEM = "apns_tokens";
static const char fb_esp_fcm_pgm_str_41[] PROGMEM = "topic";
static const char fb_esp_fcm_pgm_str_42[] PROGMEM = "image";
static const char fb_esp_fcm_pgm_str_43[] PROGMEM = "fcm_options";
static const char fb_esp_fcm_pgm_str_44[] PROGMEM = "analytics_label";
static const char fb_esp_fcm_pgm_str_45[] PROGMEM = "ttl";
static const char fb_esp_fcm_pgm_str_46[] PROGMEM = "channel_id";
static const char fb_esp_fcm_pgm_str_47[] PROGMEM = "ticker";
static const char fb_esp_fcm_pgm_str_48[] PROGMEM = "sticky";
static const char fb_esp_fcm_pgm_str_49[] PROGMEM = "event_time";
static const char fb_esp_fcm_pgm_str_50[] PROGMEM = "local_only";
static const char fb_esp_fcm_pgm_str_51[] PROGMEM = "notification_priority";
static const char fb_esp_fcm_pgm_str_52[] PROGMEM = "default_sound";
static const char fb_esp_fcm_pgm_str_53[] PROGMEM = "default_vibrate_timings";
static const char fb_esp_fcm_pgm_str_54[] PROGMEM = "default_light_settings";
static const char fb_esp_fcm_pgm_str_55[] PROGMEM = "vibrate_timings";
static const char fb_esp_fcm_pgm_str_56[] PROGMEM = "visibility";
static const char fb_esp_fcm_pgm_str_57[] PROGMEM = "notification_count";
static const char fb_esp_fcm_pgm_str_58[] PROGMEM = "light_settings";
static const char fb_esp_fcm_pgm_str_59[] PROGMEM = "red";
static const char fb_esp_fcm_pgm_str_60[] PROGMEM = "green";
static const char fb_esp_fcm_pgm_str_61[] PROGMEM = "blue";
static const char fb_esp_fcm_pgm_str_62[] PROGMEM = "alpha";
static const char fb_esp_fcm_pgm_str_63[] PROGMEM = "light_on_duration";
static const char fb_esp_fcm_pgm_str_64[] PROGMEM = "light_off_duration";
static const char fb_esp_fcm_pgm_str_65[] PROGMEM = "headers";
static const char fb_esp_fcm_pgm_str_66[] PROGMEM = "link";
#endif

// Commonly used for legacy FCM
static const char fb_esp_fcm_pgm_str_67[] PROGMEM = "notification";
static const char fb_esp_fcm_pgm_str_68[] PROGMEM = "message";
static const char fb_esp_fcm_pgm_str_69[] PROGMEM = "android";
static const char fb_esp_fcm_pgm_str_70[] PROGMEM = "webpush";
static const char fb_esp_fcm_pgm_str_71[] PROGMEM = "apns";
#endif

// Firestore class string
#if defined(ENABLE_FIRESTORE)
static const char fb_esp_cfs_pgm_str_1[] PROGMEM = "queryScope";
static const char fb_esp_cfs_pgm_str_2[] PROGMEM = "collectionIds";
static const char fb_esp_cfs_pgm_str_3[] PROGMEM = "outputUriPrefix";
static const char fb_esp_cfs_pgm_str_4[] PROGMEM = "inputUriPrefix";
static const char fb_esp_cfs_pgm_str_5[] PROGMEM = "fieldPath";
static const char fb_esp_cfs_pgm_str_6[] PROGMEM = "setToServerValue";
static const char fb_esp_cfs_pgm_str_7[] PROGMEM = "increment";
static const char fb_esp_cfs_pgm_str_8[] PROGMEM = "maximum";
static const char fb_esp_cfs_pgm_str_9[] PROGMEM = "minimum";
static const char fb_esp_cfs_pgm_str_10[] PROGMEM = "appendMissingElements";
static const char fb_esp_cfs_pgm_str_11[] PROGMEM = "removeAllFromArray";
static const char fb_esp_cfs_pgm_str_12[] PROGMEM = "fieldPaths";
static const char fb_esp_cfs_pgm_str_13[] PROGMEM = "updateTransforms";
static const char fb_esp_cfs_pgm_str_14[] PROGMEM = "exists";
static const char fb_esp_cfs_pgm_str_15[] PROGMEM = "updateTime";
static const char fb_esp_cfs_pgm_str_16[] PROGMEM = "currentDocument";
static const char fb_esp_cfs_pgm_str_17[] PROGMEM = "document";
static const char fb_esp_cfs_pgm_str_18[] PROGMEM = "fieldTransforms";
static const char fb_esp_cfs_pgm_str_19[] PROGMEM = "transform";
static const char fb_esp_cfs_pgm_str_20[] PROGMEM = "writes";
static const char fb_esp_cfs_pgm_str_21[] PROGMEM = "/documents";
static const char fb_esp_cfs_pgm_str_22[] PROGMEM = "mask";
static const char fb_esp_cfs_pgm_str_23[] PROGMEM = "newTransaction";
static const char fb_esp_cfs_pgm_str_24[] PROGMEM = "readTime";
static const char fb_esp_cfs_pgm_str_25[] PROGMEM = "options/readOnly/readTime";
static const char fb_esp_cfs_pgm_str_26[] PROGMEM = "options/readWrite/retryTransaction";
static const char fb_esp_cfs_pgm_str_27[] PROGMEM = "structuredQuery";
static const char fb_esp_cfs_pgm_str_28[] PROGMEM = "transaction";
static const char fb_esp_cfs_pgm_str_29[] PROGMEM = "newTransaction";
static const char fb_esp_cfs_pgm_str_30[] PROGMEM = "apiScope";
static const char fb_esp_cfs_pgm_str_31[] PROGMEM = "fields";
static const char fb_esp_cfs_pgm_str_32[] PROGMEM = "/databases/";
static const char fb_esp_cfs_pgm_str_33[] PROGMEM = "(default)";
static const char fb_esp_cfs_pgm_str_34[] PROGMEM = ":exportDocuments";
static const char fb_esp_cfs_pgm_str_35[] PROGMEM = ":importDocuments";
static const char fb_esp_cfs_pgm_str_36[] PROGMEM = ":beginTransaction";
static const char fb_esp_cfs_pgm_str_37[] PROGMEM = ":rollback";
static const char fb_esp_cfs_pgm_str_38[] PROGMEM = ":batchGet";
static const char fb_esp_cfs_pgm_str_39[] PROGMEM = ":batchWrite";
static const char fb_esp_cfs_pgm_str_40[] PROGMEM = "documentId=";
static const char fb_esp_cfs_pgm_str_41[] PROGMEM = ":listCollectionIds";
static const char fb_esp_cfs_pgm_str_42[] PROGMEM = ":runQuery";
static const char fb_esp_cfs_pgm_str_43[] PROGMEM = "orderBy=";
static const char fb_esp_cfs_pgm_str_44[] PROGMEM = "showMissing=";
static const char fb_esp_cfs_pgm_str_45[] PROGMEM = "updateMask.fieldPaths=";
static const char fb_esp_cfs_pgm_str_46[] PROGMEM = "currentDocument.exists=";
static const char fb_esp_cfs_pgm_str_47[] PROGMEM = "currentDocument.updateTime=";
static const char fb_esp_cfs_pgm_str_48[] PROGMEM = ":commit";
static const char fb_esp_cfs_pgm_str_49[] PROGMEM = "mask.fieldPaths=";
static const char fb_esp_cfs_pgm_str_50[] PROGMEM = "transaction=";
static const char fb_esp_cfs_pgm_str_51[] PROGMEM = "readTime=";
static const char fb_esp_cfs_pgm_str_52[] PROGMEM = "/collectionGroups/";
static const char fb_esp_cfs_pgm_str_53[] PROGMEM = "/indexes";
static const char fb_esp_cfs_pgm_str_54[] PROGMEM = "filter";
static const char fb_esp_cfs_pgm_str_55[] PROGMEM = "firestore.";
#endif

// Firebase Storage class string
#if defined(ENABLE_FB_STORAGE)
static const char fb_esp_storage_pgm_str_1[] PROGMEM = "name=";
#endif

// Google Cloud Storage class string
#if defined(ENABLE_GC_STORAGE)
static const char fb_esp_gcs_pgm_str_1[] PROGMEM = "/storage/v1/b/";
static const char fb_esp_gcs_pgm_str_2[] PROGMEM = "/upload";
static const char fb_esp_gcs_pgm_str_3[] PROGMEM = "/o";
static const char fb_esp_gcs_pgm_str_4[] PROGMEM = "?alt=media";
static const char fb_esp_gcs_pgm_str_5[] PROGMEM = "?uploadType=media&name=";
static const char fb_esp_gcs_pgm_str_6[] PROGMEM = "?uploadType=multipart";
static const char fb_esp_gcs_pgm_str_7[] PROGMEM = "?uploadType=resumable&name=";
static const char fb_esp_gcs_pgm_str_8[] PROGMEM = "?alt=json";
static const char fb_esp_gcs_pgm_str_9[] PROGMEM = "X-Upload-Content-Type: ";
static const char fb_esp_gcs_pgm_str_10[] PROGMEM = "X-Upload-Content-Length: ";
static const char fb_esp_gcs_pgm_str_11[] PROGMEM = "Content-Range: bytes ";
static const char fb_esp_gcs_pgm_str_12[] PROGMEM = "Content-Type: application/json; charset=UTF-8\r\n";
static const char fb_esp_gcs_pgm_str_13[] PROGMEM = "--";
static const char fb_esp_gcs_pgm_str_14[] PROGMEM = "multipart/related; boundary=";
static const char fb_esp_gcs_pgm_str_15[] PROGMEM = "generation=";
static const char fb_esp_gcs_pgm_str_16[] PROGMEM = "ifGenerationMatch=";
static const char fb_esp_gcs_pgm_str_17[] PROGMEM = "ifGenerationNotMatch=";
static const char fb_esp_gcs_pgm_str_18[] PROGMEM = "ifMetagenerationMatch=";
static const char fb_esp_gcs_pgm_str_19[] PROGMEM = "ifMetagenerationNotMatch=";
static const char fb_esp_gcs_pgm_str_20[] PROGMEM = "contentEncoding=";
static const char fb_esp_gcs_pgm_str_21[] PROGMEM = "projection=";
static const char fb_esp_gcs_pgm_str_22[] PROGMEM = "kmsKeyName=";
static const char fb_esp_gcs_pgm_str_23[] PROGMEM = "predefinedAcl=";
static const char fb_esp_gcs_pgm_str_24[] PROGMEM = "firebaseStorageDownloadTokens";
static const char fb_esp_gcs_pgm_str_25[] PROGMEM = "a82781ce-a115-442f-bac6-a52f7f63b3e8";
static const char fb_esp_gcs_pgm_str_26[] PROGMEM = "acl";
static const char fb_esp_gcs_pgm_str_27[] PROGMEM = "cacheControl";
static const char fb_esp_gcs_pgm_str_28[] PROGMEM = "contentDisposition";
static const char fb_esp_gcs_pgm_str_29[] PROGMEM = "contentEncoding";
static const char fb_esp_gcs_pgm_str_30[] PROGMEM = "contentLanguage";
static const char fb_esp_gcs_pgm_str_31[] PROGMEM = "contentType";
static const char fb_esp_gcs_pgm_str_32[] PROGMEM = "crc32c";
static const char fb_esp_gcs_pgm_str_33[] PROGMEM = "customTime";
static const char fb_esp_gcs_pgm_str_34[] PROGMEM = "eventBasedHold";
static const char fb_esp_gcs_pgm_str_35[] PROGMEM = "md5Hash";
static const char fb_esp_gcs_pgm_str_36[] PROGMEM = "metadata";
static const char fb_esp_gcs_pgm_str_37[] PROGMEM = "name";
static const char fb_esp_gcs_pgm_str_38[] PROGMEM = "storageClass";
static const char fb_esp_gcs_pgm_str_39[] PROGMEM = "temporaryHold";
static const char fb_esp_gcs_pgm_str_40[] PROGMEM = "maxResults=";
static const char fb_esp_gcs_pgm_str_41[] PROGMEM = "delimiter=";
static const char fb_esp_gcs_pgm_str_42[] PROGMEM = "endOffset=";
static const char fb_esp_gcs_pgm_str_43[] PROGMEM = "includeTrailingDelimiter=";
static const char fb_esp_gcs_pgm_str_44[] PROGMEM = "prefix=";
static const char fb_esp_gcs_pgm_str_45[] PROGMEM = "startOffset=";
static const char fb_esp_gcs_pgm_str_46[] PROGMEM = "versions=";
static const char fb_esp_gcs_pgm_str_47[] PROGMEM = "resumableUploadTask";
static const char fb_esp_gcs_pgm_str_48[] PROGMEM = "Range: bytes=0-";
#endif

// Firebase Functions class string
#if defined(ENABLE_FB_FUNCTIONS)
static const char fb_esp_func_pgm_str_1[] PROGMEM = "deployTask";
static const char fb_esp_func_pgm_str_2[] PROGMEM = "projectId";
static const char fb_esp_func_pgm_str_3[] PROGMEM = "location";
static const char fb_esp_func_pgm_str_4[] PROGMEM = "autozip";
static const char fb_esp_func_pgm_str_5[] PROGMEM = "zip";
static const char fb_esp_func_pgm_str_6[] PROGMEM = "accessToken";
static const char fb_esp_func_pgm_str_7[] PROGMEM = "path";
static const char fb_esp_func_pgm_str_8[] PROGMEM = "tmp.zip";
static const char fb_esp_func_pgm_str_9[] PROGMEM = "databaseURL";
static const char fb_esp_func_pgm_str_10[] PROGMEM = "storageBucket";
static const char fb_esp_func_pgm_str_11[] PROGMEM = "locationId";
static const char fb_esp_func_pgm_str_12[] PROGMEM = "environmentVariables";
static const char fb_esp_func_pgm_str_13[] PROGMEM = "FIREBASE_CONFIG";
static const char fb_esp_func_pgm_str_14[] PROGMEM = "GCLOUD_PROJECT";
static const char fb_esp_func_pgm_str_15[] PROGMEM = "httpsTrigger/url";
static const char fb_esp_func_pgm_str_16[] PROGMEM = "policy";
static const char fb_esp_func_pgm_str_17[] PROGMEM = "status";
static const char fb_esp_func_pgm_str_18[] PROGMEM = "CLOUD_FUNCTION_STATUS_UNSPECIFIED";
static const char fb_esp_func_pgm_str_19[] PROGMEM = "ACTIVE";
static const char fb_esp_func_pgm_str_20[] PROGMEM = "OFFLINE";
static const char fb_esp_func_pgm_str_21[] PROGMEM = "DEPLOY_IN_PROGRESS";
static const char fb_esp_func_pgm_str_22[] PROGMEM = "DELETE_IN_PROGRESS";
static const char fb_esp_func_pgm_str_23[] PROGMEM = "UNKNOWN";
static const char fb_esp_func_pgm_str_24[] PROGMEM = "versionId";
static const char fb_esp_func_pgm_str_25[] PROGMEM = "/v1/operations";
static const char fb_esp_func_pgm_str_26[] PROGMEM = "filter=";
static const char fb_esp_func_pgm_str_27[] PROGMEM = "/locations/";
static const char fb_esp_func_pgm_str_28[] PROGMEM = "/functions";
static const char fb_esp_func_pgm_str_29[] PROGMEM = ":call";
static const char fb_esp_func_pgm_str_30[] PROGMEM = ":setIamPolicy";
static const char fb_esp_func_pgm_str_31[] PROGMEM = ":generateDownloadUrl";
static const char fb_esp_func_pgm_str_32[] PROGMEM = ":getIamPolicy";
static const char fb_esp_func_pgm_str_33[] PROGMEM = "options.requestedPolicyVersion";
static const char fb_esp_func_pgm_str_34[] PROGMEM = "updateMask=";
static const char fb_esp_func_pgm_str_35[] PROGMEM = ":generateUploadUrl";
static const char fb_esp_func_pgm_str_36[] PROGMEM = "application/zip";
static const char fb_esp_func_pgm_str_37[] PROGMEM = "x-goog-content-length-range: 0,104857600";
static const char fb_esp_func_pgm_str_38[] PROGMEM = "cloudfunctions.";
static const char fb_esp_func_pgm_str_39[] PROGMEM = "operations/[0]/error/code";
static const char fb_esp_func_pgm_str_40[] PROGMEM = "operations/[0]/error/message";
static const char fb_esp_func_pgm_str_41[] PROGMEM = "error/details";
static const char fb_esp_func_pgm_str_42[] PROGMEM = "uploadUrl";
static const char fb_esp_func_pgm_str_43[] PROGMEM = "status/uploadUrl";
static const char fb_esp_func_pgm_str_44[] PROGMEM = "sourceUploadUrl";
static const char fb_esp_func_pgm_str_45[] PROGMEM = "project:";
static const char fb_esp_func_pgm_str_46[] PROGMEM = ",latest:true";
static const char fb_esp_func_pgm_str_47[] PROGMEM = "projects/";
static const char fb_esp_func_pgm_str_48[] PROGMEM = "description";
static const char fb_esp_func_pgm_str_49[] PROGMEM = "entryPoint";
static const char fb_esp_func_pgm_str_50[] PROGMEM = "runtime";
static const char fb_esp_func_pgm_str_51[] PROGMEM = "s";
static const char fb_esp_func_pgm_str_52[] PROGMEM = "timeout";
static const char fb_esp_func_pgm_str_53[] PROGMEM = "availableMemoryMb";
static const char fb_esp_func_pgm_str_54[] PROGMEM = "maxInstances";
static const char fb_esp_func_pgm_str_55[] PROGMEM = "vpcConnector";
static const char fb_esp_func_pgm_str_56[] PROGMEM = "vpcConnectorEgressSettings";
static const char fb_esp_func_pgm_str_57[] PROGMEM = "ingressSettings";
static const char fb_esp_func_pgm_str_58[] PROGMEM = "sourceArchiveUrl";
static const char fb_esp_func_pgm_str_59[] PROGMEM = "sourceRepository";
static const char fb_esp_func_pgm_str_60[] PROGMEM = "buildEnvironmentVariables";
static const char fb_esp_func_pgm_str_61[] PROGMEM = "network";
static const char fb_esp_func_pgm_str_62[] PROGMEM = "eventTrigger/eventType";
static const char fb_esp_func_pgm_str_63[] PROGMEM = "eventTrigger/resource";
static const char fb_esp_func_pgm_str_64[] PROGMEM = "eventTrigger/service";
static const char fb_esp_func_pgm_str_65[] PROGMEM = "eventTrigger/failurePolicy";
static const char fb_esp_func_pgm_str_66[] PROGMEM = "{\"retry\":{}}";
static const char fb_esp_func_pgm_str_67[] PROGMEM = "eventTrigger";
static const char fb_esp_func_pgm_str_68[] PROGMEM = "policy";
static const char fb_esp_func_pgm_str_69[] PROGMEM = "auditConfigs";
static const char fb_esp_func_pgm_str_70[] PROGMEM = "etag";
static const char fb_esp_func_pgm_str_71[] PROGMEM = "logType";
static const char fb_esp_func_pgm_str_72[] PROGMEM = "exemptedMembers";
static const char fb_esp_func_pgm_str_73[] PROGMEM = "service";
static const char fb_esp_func_pgm_str_74[] PROGMEM = "role";
static const char fb_esp_func_pgm_str_75[] PROGMEM = "members";
static const char fb_esp_func_pgm_str_76[] PROGMEM = "bindings";
static const char fb_esp_func_pgm_str_77[] PROGMEM = "condition";
static const char fb_esp_func_pgm_str_78[] PROGMEM = "expression";
static const char fb_esp_func_pgm_str_79[] PROGMEM = "title";
static const char fb_esp_func_pgm_str_80[] PROGMEM = "description";
static const char fb_esp_func_pgm_str_81[] PROGMEM = "version";
static const char fb_esp_func_pgm_str_82[] PROGMEM = ".cloudfunctions.net";
#endif

// Session class string
#if defined(ENABLE_RTDB)
static const char fb_esp_rtdb_ss_pgm_str_1[] PROGMEM = "json";
static const char fb_esp_rtdb_ss_pgm_str_2[] PROGMEM = "string";
static const char fb_esp_rtdb_ss_pgm_str_3[] PROGMEM = "array";
static const char fb_esp_rtdb_ss_pgm_str_4[] PROGMEM = "float";
static const char fb_esp_rtdb_ss_pgm_str_5[] PROGMEM = "int";
static const char fb_esp_rtdb_ss_pgm_str_6[] PROGMEM = "null";
static const char fb_esp_rtdb_ss_pgm_str_7[] PROGMEM = "double";
static const char fb_esp_rtdb_ss_pgm_str_8[] PROGMEM = "boolean";
static const char fb_esp_rtdb_ss_pgm_str_9[] PROGMEM = "blob";
static const char fb_esp_rtdb_ss_pgm_str_10[] PROGMEM = "file";
static const char fb_esp_rtdb_ss_pgm_str_11[] PROGMEM = "get";
static const char fb_esp_rtdb_ss_pgm_str_12[] PROGMEM = "set";
static const char fb_esp_rtdb_ss_pgm_str_13[] PROGMEM = "push";
static const char fb_esp_rtdb_ss_pgm_str_14[] PROGMEM = "task";
static const char fb_esp_rtdb_ss_pgm_str_15[] PROGMEM = "_stream";
static const char fb_esp_rtdb_ss_pgm_str_16[] PROGMEM = "_error_queue";
#endif

// Storage classes string
#if defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE)
static const char fb_esp_storage_ss_pgm_str_1[] PROGMEM = "firebasestorage.";
static const char fb_esp_storage_ss_pgm_str_2[] PROGMEM = "/v0/b/";
static const char fb_esp_storage_ss_pgm_str_3[] PROGMEM = "/o";
static const char fb_esp_storage_ss_pgm_str_4[] PROGMEM = "alt=media";
static const char fb_esp_storage_ss_pgm_str_5[] PROGMEM = "token=";
static const char fb_esp_storage_ss_pgm_str_6[] PROGMEM = "bucket";
static const char fb_esp_storage_ss_pgm_str_7[] PROGMEM = "generation";
static const char fb_esp_storage_ss_pgm_str_8[] PROGMEM = "metageneration";
static const char fb_esp_storage_ss_pgm_str_9[] PROGMEM = "contentType";
static const char fb_esp_storage_ss_pgm_str_10[] PROGMEM = "size";
static const char fb_esp_storage_ss_pgm_str_11[] PROGMEM = "crc32c";
static const char fb_esp_storage_ss_pgm_str_12[] PROGMEM = "etag";
static const char fb_esp_storage_ss_pgm_str_13[] PROGMEM = "downloadTokens";
static const char fb_esp_storage_ss_pgm_str_14[] PROGMEM = "metadata/firebaseStorageDownloadTokens";
static const char fb_esp_storage_ss_pgm_str_15[] PROGMEM = "mediaLink";
#endif

static const char fb_esp_storage_ss_pgm_str_16[] PROGMEM = "error/code";
static const char fb_esp_storage_ss_pgm_str_17[] PROGMEM = "error/message";

/////////////////////////////////////////
// Error string

#if defined(ENABLE_ERROR_STRING)
// General error string
static const char fb_esp_general_err_pgm_str_1[] PROGMEM = "unknown error";
static const char fb_esp_general_err_pgm_str_2[] PROGMEM = "operation ignored due to long running task is being processed.";
static const char fb_esp_general_err_pgm_str_3[] PROGMEM = "missing data.";

// Client error string
static const char fb_esp_client_err_pgm_str_1[] PROGMEM = "response payload read timed out";
static const char fb_esp_client_err_pgm_str_2[] PROGMEM = "connection refused";
static const char fb_esp_client_err_pgm_str_3[] PROGMEM = "send request failed";
static const char fb_esp_client_err_pgm_str_4[] PROGMEM = "not connected";
static const char fb_esp_client_err_pgm_str_5[] PROGMEM = "connection lost";
static const char fb_esp_client_err_pgm_str_6[] PROGMEM = "no http server";
static const char fb_esp_client_err_pgm_str_7[] PROGMEM = "response read failed.";
static const char fb_esp_client_err_pgm_str_8[] PROGMEM = "http connection was used by other processes";
static const char fb_esp_client_err_pgm_str_9[] PROGMEM = "maximum Redirection reached";
static const char fb_esp_client_err_pgm_str_10[] PROGMEM = "upload timed out";
static const char fb_esp_client_err_pgm_str_11[] PROGMEM = "upload data sent error";
static const char fb_esp_client_err_pgm_str_12[] PROGMEM = "custom Client is not yet enabled";
static const char fb_esp_client_err_pgm_str_13[] PROGMEM = "Client is not yet initialized";

// HTTP error string
static const char fb_esp_http_err_pgm_str_1[] PROGMEM = "bad request";
static const char fb_esp_http_err_pgm_str_2[] PROGMEM = "non-authoriative information";
static const char fb_esp_http_err_pgm_str_3[] PROGMEM = "no content";
static const char fb_esp_http_err_pgm_str_4[] PROGMEM = "moved permanently";
static const char fb_esp_http_err_pgm_str_5[] PROGMEM = "use proxy";
static const char fb_esp_http_err_pgm_str_6[] PROGMEM = "temporary redirect";
static const char fb_esp_http_err_pgm_str_7[] PROGMEM = "permanent redirect";
static const char fb_esp_http_err_pgm_str_8[] PROGMEM = "unauthorized";
static const char fb_esp_http_err_pgm_str_9[] PROGMEM = "forbidden";
static const char fb_esp_http_err_pgm_str_10[] PROGMEM = "not found";
static const char fb_esp_http_err_pgm_str_11[] PROGMEM = "method not allow";
static const char fb_esp_http_err_pgm_str_12[] PROGMEM = "not acceptable";
static const char fb_esp_http_err_pgm_str_13[] PROGMEM = "proxy authentication required";
static const char fb_esp_http_err_pgm_str_14[] PROGMEM = "request timed out";
static const char fb_esp_http_err_pgm_str_15[] PROGMEM = "length required";
static const char fb_esp_http_err_pgm_str_16[] PROGMEM = "too many requests";
static const char fb_esp_http_err_pgm_str_17[] PROGMEM = "request header fields too larg";
static const char fb_esp_http_err_pgm_str_18[] PROGMEM = "internal server error";
static const char fb_esp_http_err_pgm_str_19[] PROGMEM = "bad gateway";
static const char fb_esp_http_err_pgm_str_20[] PROGMEM = "service unavailable";
static const char fb_esp_http_err_pgm_str_21[] PROGMEM = "gateway timeout";
static const char fb_esp_http_err_pgm_str_22[] PROGMEM = "http version not support";
static const char fb_esp_http_err_pgm_str_23[] PROGMEM = "network authentication required";
static const char fb_esp_http_err_pgm_str_24[] PROGMEM = "precondition failed (ETag does not match)";

// Auth error string
static const char fb_esp_auth_err_pgm_str_1[] PROGMEM = "token is not ready (revoked or expired)";
static const char fb_esp_auth_err_pgm_str_2[] PROGMEM = "Firebase authentication was not initialized";
static const char fb_esp_auth_err_pgm_str_3[] PROGMEM = "missing required credentials e.g., database URL, host and tokens.";
static const char fb_esp_auth_err_pgm_str_4[] PROGMEM = "RSA private key parsing failed";
static const char fb_esp_auth_err_pgm_str_5[] PROGMEM = "JWT token signing failed";
static const char fb_esp_auth_err_pgm_str_6[] PROGMEM = "OAuth2.0 authentication required";
static const char fb_esp_auth_err_pgm_str_7[] PROGMEM = "token exchange failed";
static const char fb_esp_auth_err_pgm_str_8[] PROGMEM = "create message digest failed";

// OTA error string
#if defined(ENABLE_OTA_FIRMWARE_UPDATE)
static const char fb_esp_ota_err_pgm_str_1[] PROGMEM = "Bin size does not fit the free flash space";
static const char fb_esp_ota_err_pgm_str_2[] PROGMEM = "Updater begin() failed";
static const char fb_esp_ota_err_pgm_str_3[] PROGMEM = "Updater write() failed.";
static const char fb_esp_ota_err_pgm_str_4[] PROGMEM = "Updater end() failed.";
static const char fb_esp_ota_err_pgm_str_5[] PROGMEM = "invalid Firmware";
static const char fb_esp_ota_err_pgm_str_6[] PROGMEM = "too low free sketch space";
#endif

// Storage error string
#if defined(MBFS_FLASH_FS) || defined(MBFS_SD_FS)
static const char fb_esp_storage_err_pgm_str_1[] PROGMEM = "file I/O error";
static const char fb_esp_storage_err_pgm_str_2[] PROGMEM = "flash Storage is not ready.";
static const char fb_esp_storage_err_pgm_str_3[] PROGMEM = "SD Storage is not ready.";
static const char fb_esp_storage_err_pgm_str_4[] PROGMEM = "file is still opened.";
static const char fb_esp_storage_err_pgm_str_5[] PROGMEM = "file not found.";
#endif

// Mem error string
static const char fb_esp_mem_err_pgm_str_1[] PROGMEM = "data buffer overflow";
static const char fb_esp_mem_err_pgm_str_2[] PROGMEM = "payload too large";

// SSL error string
static const char fb_esp_ssl_err_pgm_str_1[] PROGMEM = "incomplete SSL client data";

// Time error string
static const char fb_esp_time_err_pgm_str_1[] PROGMEM = "system time was not set";
static const char fb_esp_time_err_pgm_str_2[] PROGMEM = "cannot config time";
static const char fb_esp_time_err_pgm_str_3[] PROGMEM = "device time was not set";
static const char fb_esp_time_err_pgm_str_4[] PROGMEM = "NTP server time synching failed";
static const char fb_esp_time_err_pgm_str_5[] PROGMEM = "UDP client is required for NTP server time synching based on your network type \ne.g. WiFiUDP or EthernetUDP. Please call Firebase.setUDPClient(&udpClient, gmtOffset); to assign the UDP client.";

// RTDB error string
static const char fb_esp_rtdb_err_pgm_str_1[] PROGMEM = "backup data should be the JSON object";
static const char fb_esp_rtdb_err_pgm_str_2[] PROGMEM = "path not exist";
static const char fb_esp_rtdb_err_pgm_str_3[] PROGMEM = "data type mismatch";
static const char fb_esp_rtdb_err_pgm_str_4[] PROGMEM = "security rules is not a valid JSON";


// FCM error string
static const char fb_esp_fcm_err_pgm_str_1[] PROGMEM = "no ID token or registration token provided";
static const char fb_esp_fcm_err_pgm_str_2[] PROGMEM = "mo server key provided";
static const char fb_esp_fcm_err_pgm_str_3[] PROGMEM = "no topic provided";
static const char fb_esp_fcm_err_pgm_str_4[] PROGMEM = "ID token or registration token was not not found at index";

#endif

// Firebase Functions error string
#if defined(ENABLE_FB_FUNCTIONS)
static const char fb_esp_functions_err_pgm_str_1[] PROGMEM = "missing autozip function, please deploy it first";
#endif

static const char fb_esp_boundary_table[] PROGMEM = "=_abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const unsigned char fb_esp_base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#endif