#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase Realtime Database class, FB_RTDB.h version 2.0.14
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

#ifdef ENABLE_RTDB

#ifndef FIREBASE_RTDB_H
#define FIREBASE_RTDB_H

#include "FB_Utils.h"
#include "session/FB_Session.h"
#include "QueueInfo.h"
#include "stream/FB_MP_Stream.h"
#include "stream/FB_Stream.h"

using namespace mb_string;

class FB_RTDB
{

#if defined(FIREBASE_ESP8266_CLIENT) || defined(FIREBASE_ESP32_CLIENT)
  friend class FIREBASE_CLASS;
#elif defined(FIREBASE_ESP_CLIENT)
  friend class Firebase_ESP_Client;
#endif

#if defined(ENABLE_ERROR_QUEUE)
#if !defined(ESP32) && !defined(ESP8266) && !defined(MB_ARDUINO_PICO)
#undef ENABLE_ERROR_QUEUE
#endif
#endif

public:
  FB_RTDB();
  ~FB_RTDB();

  /** Stop Firebase and release all resources.
   *
   * @param fbdo The pointer to Firebase Data Object.
   */
  void end(FirebaseData *fbdo);

#if defined(ESP32) || defined(MB_ARDUINO_PICO)
  /** Enable multiple HTTP requests at a time (for ESP32 only).
   *
   * @param enable - The boolean value to enable/disable.
   *
   * @note The multiple HTTP requessts at a time is disable by default to prevent the large memory used in multiple requests.
   */
  void allowMultipleRequests(bool enable);
#endif

  /** Set the timeout of Firebase.get functions.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param millisec The milliseconds to limit the request (0 900,000 ms or 15 min).
   */
  template <typename T = int>
  void setReadTimeout(FirebaseData *fbdo, T millisec) { mSetReadTimeout(fbdo, toStringPtr(millisec, -1)); }

  /** Set the size limit of payload data that will write to the database for each request.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param size The size identified string e.g. tiny, small, medium, large and unlimited.
   *
   * @note Size string and its write timeout in seconds e.g. tiny (1s), small (10s), medium (30s) and large (60s).
   */
  template <typename T = const char *>
  void setwriteSizeLimit(FirebaseData *fbdo, T size) { return mSetwriteSizeLimit(fbdo, toStringPtr(size)); }

  /** Read the database rules.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].jsonData will return the JSON string value of
   * database rules returned from the server.
   */
  bool getRules(FirebaseData *fbdo) { return mGetRules(fbdo, mem_storage_type_undefined, toStringPtr(_EMPTY_STR), NULL); }

  /** Save the database rules to file.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param filename Filename to save rules.
   * @param callback Optional. The callback function that accept RTDB_DownloadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].jsonData will return the JSON string value of
   * database rules returned from the server.
   */
  template <typename T = const char *>
  bool getRules(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T filename, RTDB_DownloadProgressCallback callback = NULL) { return mGetRules(fbdo, storageType, toStringPtr(filename), callback); }

  /** Write the database rules.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param rules The JSON serialized string of the rules.
   * @return Boolean value, indicates the success of the operation.
   */
  template <typename T = const char *>
  bool setRules(FirebaseData *fbdo, T rules)
  {
    return mSetRules(fbdo, toStringPtr(rules), mem_storage_type_undefined, toStringPtr(_EMPTY_STR), NULL);
  }

  /** Restore the database rules from file.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param filename Filename to read the rules from.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   */
  template <typename T = const char *>
  bool setRules(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T filename, RTDB_UploadProgressCallback callback = NULL)
  {
    return mSetRules(fbdo, toStringPtr(_EMPTY_STR), storageType, toStringPtr(filename), callback);
  }

  /** Set the .read and .write database rules.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The parent path of child's node that the .read and .write rules are being set.
   * @param var The child node key that the .read and .write rules are being set.
   * @param readVal The child node key .read value.
   * @param writeVal The child node key .write value.
   * @param databaseSecret The database secret.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type
   * is Email/Password sign-in.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *,
            typename T4 = const char *, typename T5 = const char *>
  bool setReadWriteRules(FirebaseData *fbdo, T1 path, T2 var, T3 readVal, T4 writeVal, T5 databaseSecret)
  {
    return mSetReadWriteRules(fbdo, toStringPtr(path), toStringPtr(var), toStringPtr(readVal),
                              toStringPtr(writeVal), toStringPtr(databaseSecret));
  }

  /** Set the query index to the database rules.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The parent path of child's node that is being queried.
   * @param node The child node key that is being queried.
   * @param databaseSecret The database secret.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type
   * is Email/Password sign-in.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setQueryIndex(FirebaseData *fbdo, T1 path, T2 node, T3 databaseSecret)
  {
    return mSetQueryIndex(fbdo, toStringPtr(path), toStringPtr(node), toStringPtr(databaseSecret));
  }

  /** Remove the query index from the database rules.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The parent path of child's node that the index is being removed.
   * @param databaseSecret The database secret.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type
   * is Email/Password sign-in.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool removeQueryIndex(FirebaseData *fbdo, T1 path, T2 databaseSecret)
  {
    return mSetQueryIndex(fbdo, toStringPtr(path), toStringPtr(_EMPTY_STR), toStringPtr(databaseSecret));
  }

  /** Determine the existent of the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, true if the defined node was found.
   */
  template <typename T = const char *>
  bool pathExisted(FirebaseData *fbdo, T path) { return mPathExisted(fbdo, toStringPtr(path)); }

  /** Determine the unique identifier (ETag) of current data at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return String of unique identifier.
   */
  template <typename T = const char *>
  String getETag(FirebaseData *fbdo, T path) { return mGetETag(fbdo, toStringPtr(path)); }

  /** Get the shallowed data at a defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Return the child data with the value or JSON object (the value will be truncated to true).
   * Call [FirebaseData object].to<String>() to get shallowed string (number, string and JSON object).
   */
  template <typename T = const char *>
  bool getShallowData(FirebaseData *fbdo, T path) { return mGetShallowData(fbdo, toStringPtr(path)); }

  /** Enable the library to use only classic HTTP GET and POST methods.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param flag Boolean value, true to enable, false to disable.
   *
   * @note This option used to escape the Firewall restriction (if the device is connected through
   * Firewall) that allows only HTTP GET and POST
   * HTTP PATCH request was sent as PATCH which not affected by this option.
   */
  void enableClassicRequest(FirebaseData *fbdo, bool enable);

  /** Set the virtual child node ".priority" to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param priority The priority value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note This allows us to set priority to any node other than a priority that set through setJSON,
   * pushJSON, updateNode, and updateNodeSilent functions.
   */
  template <typename T = const char *>
  bool setPriority(FirebaseData *fbdo, T path, float &priority)
  {
    return buildRequest(fbdo, rtdb_set_priority, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority),
                        toStringPtr(_NO_ETAG), _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setPriorityAsync(FirebaseData *fbdo, T path, float &priority)
  {
    return buildRequest(fbdo, rtdb_set_priority, toStringPtr(path),
                        toStringPtr(_NO_PAYLOAD), d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY,
                        toAddr(priority), toStringPtr(_NO_ETAG), _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read the virtual child node ".priority" value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   */
  template <typename T = const char *>
  bool getPriority(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, rtdb_get_priority, toStringPtr(path),
                        toStringPtr(_NO_PAYLOAD), d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY,
                        _NO_PRIORITY, toStringPtr(_NO_ETAG), _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post)  new integer value to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param value The appended value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T1 = const char *, typename T2 = int>
  bool pushInt(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = int>
  bool pushIntAsync(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new integer value and the virtual child ".priority" to the defined node.
   */
  template <typename T1 = const char *, typename T2 = int>
  bool pushInt(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = int>
  bool pushIntAsync(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new float value to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float value will be appended.
   * @param value The appended value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T = const char *>
  bool pushFloat(FirebaseData *fbdo, T path, float value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushFloatAsync(FirebaseData *fbdo, T path, float value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new float value and the virtual child ".priority" to the defined node.
   */
  template <typename T = const char *>
  bool pushFloat(FirebaseData *fbdo, T path, float value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushFloatAsync(FirebaseData *fbdo, T path, float value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new double value (8 bytes) to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float value will be appended.
   * @param value The appended value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T = const char *>
  bool pushDouble(FirebaseData *fbdo, T path, double value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushDoubleAsync(FirebaseData *fbdo, T path, double value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new double value (8 bytes) and the virtual child ".priority" to the defined node.
   */
  template <typename T = const char *>
  bool pushDouble(FirebaseData *fbdo, T path, double value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushDoubleAsync(FirebaseData *fbdo, T path, double value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new Boolean value to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which Boolean value will be appended.
   * @param value The appended value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.

  */
  template <typename T = const char *>
  bool pushBool(FirebaseData *fbdo, T path, bool value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushBoolAsync(FirebaseData *fbdo, T path, bool value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) the new Boolean value and the virtual child ".priority" to the defined node.
   */
  template <typename T = const char *>
  bool pushBool(FirebaseData *fbdo, T path, bool value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushBoolAsync(FirebaseData *fbdo, T path, bool value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) a new string (text) to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which string will be appended.
   * @param value The appended value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool pushString(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushStringAsync(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new string and the virtual child ".priority" to the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool pushString(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushStringAsync(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new child (s) to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which child (s) will be appended.
   * @param json The pointer to the FirebaseJson object which contains the child (s) nodes.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T = const char *>
  bool pushJSON(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushJSONAsync(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new child (s) and the virtual child ".priority" to the defined node.
   */
  template <typename T = const char *>
  bool pushJSON(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushJSONAsync(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) array to the defined node.
   * The old content in defined node will be replaced.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which the array will be appended.
   * @param arr The pointer to the FirebaseJsonArray object.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T = const char *>
  bool pushArray(FirebaseData *fbdo, T path, FirebaseJsonArray *arr)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushArrayAsync(FirebaseData *fbdo, T path, FirebaseJsonArray *arr)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) array and virtual child ".priority" at the defined node.
   */
  template <typename T = const char *>
  bool pushArray(FirebaseData *fbdo, T path, FirebaseJsonArray *arr, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushArrayAsync(FirebaseData *fbdo, T path, FirebaseJsonArray *arr, float &priority)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Append (post) new blob (binary data) to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which binary data will be appended.The path to the node in which binary data will be appended.
   * @param blob Byte array of data.
   * @param size Size of the byte array.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T = const char *>
  bool pushBlob(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, toAddr(blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, size, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushBlobAsync(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, toAddr(blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, size, toStringPtr(_NO_FILE));
  }

  /** Append (post) new binary data from file stores on storage memory to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param path The path to the node in which binary data will be appended.
   * @param fileName The file path includes its name.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName,
                RTDB_UploadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_file, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(fileName), storageType, NULL, callback);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path,
                     T2 fileName, RTDB_UploadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_file, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(fileName), storageType, NULL, callback);
  }

  /** Append (post) the new Firebase server's timestamp to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which timestamp will be appended.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The key or name of new created node will be stored in Firebase Data object,
   * call [FirebaseData object].pushName() to get the key.
   */
  template <typename T = const char *>
  bool pushTimestamp(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path),
                        toStringPtr(pgm2Str(fb_esp_rtdb_pgm_str_39 /* "{\".sv\": \"timestamp\"}" */)),
                        d_timestamp, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool pushTimestampAsync(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_post, toStringPtr(path),
                        toStringPtr(pgm2Str(fb_esp_rtdb_pgm_str_39 /* "{\".sv\": \"timestamp\"}" */)),
                        d_timestamp, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the integer value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node which integer value will be set.
   * @param value Integer value to set.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<int>() to get the integer value that stored on the defined node.
   */
  template <typename T1 = const char *, typename T2 = int>
  bool setInt(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = int>
  bool setIntAsync(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the integer value and virtual child ".priority" at the defined node.
   */
  template <typename T1 = const char *, typename T2 = int>
  bool setInt(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = int>
  bool setIntAsync(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the integer value at the defined node if defined node's ETag matched the defined ETag value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which integer value will be set.
   * @param value Integer value to set.
   * @param ETag Known unique identifier string (ETag) of the defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<int>() to get the integer value that stored on the defined node.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].to<int>() to get the current integer value.
   */
  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setInt(FirebaseData *fbdo, T1 path, T2 value, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setIntAsync(FirebaseData *fbdo, T1 path, T2 value, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) integer value and the virtual child ".priority" if defined ETag matches at the defined node
   */
  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setInt(FirebaseData *fbdo, T1 path, T2 value, float &priority, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setIntAsync(FirebaseData *fbdo, T1 path, T2 value, float &priority, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) float value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float value will be set.
   * @param value Float value to set.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<float>() to get the float value that stored on the defined node.
   */
  template <typename T = const char *>
  bool setFloat(FirebaseData *fbdo, T path, float value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setFloatAsync(FirebaseData *fbdo, T path, float value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) float value and virtual child ".priority" at the defined node.
   */
  template <typename T = const char *>
  bool setFloat(FirebaseData *fbdo, T path, float value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setFloatAsync(FirebaseData *fbdo, T path, float value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) float value at the defined node if defined node's ETag matched the ETag value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float data will be set.
   * @param value Float value to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<float>() to get the float value that stored on the defined node.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].to<float>() to get the current float value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloat(FirebaseData *fbdo, T1 path, float value, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloatAsync(FirebaseData *fbdo, T1 path, float value, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) float value and the virtual child ".priority" if defined ETag matches at the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloat(FirebaseData *fbdo, T1 path, float value, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloatAsync(FirebaseData *fbdo, T1 path, float value, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(false)),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) double value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float data will be set.
   * @param value Double value to set.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<double>() to get the double value that stored on the defined node.
   *
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place,
   * use printf("%.9lf\n", firebaseData.to<double>()); for print the returned double value up to 9 decimal places.
   */
  template <typename T = const char *>
  bool setDouble(FirebaseData *fbdo, T path, double value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setDoubleAsync(FirebaseData *fbdo, T path, double value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) double value and virtual child ".priority" at the defined node.
   */
  template <typename T = const char *>
  bool setDouble(FirebaseData *fbdo, T path, double value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setDoubleAsync(FirebaseData *fbdo, T path, double value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) double value at the defined node if defined node's ETag matched the ETag value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float data will be set.
   * @param value Double value to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<double>() to get the double value that stored on the defined node.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].doubeData to get the current double value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setDouble(FirebaseData *fbdo, T1 path, double value, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setDoubleAsync(FirebaseData *fbdo, T1 path, double value, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) double value and the virtual child ".priority" if defined ETag matches at the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setDouble(FirebaseData *fbdo, T1 path, double value, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setDoubleAsync(FirebaseData *fbdo, T1 path, double value, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, getPrec(true)),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) boolean value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which Boolean data will be set.
   * @param value Boolean value to set.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<bool>() to get the integer value that stored on the defined node.
   */
  template <typename T = const char *>
  bool setBool(FirebaseData *fbdo, T path, bool value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setBoolAsync(FirebaseData *fbdo, T path, bool value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) boolean value and virtual child ".priority" at the defined node.
   */
  template <typename T = const char *>
  bool setBool(FirebaseData *fbdo, T path, bool value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setBoolAsync(FirebaseData *fbdo, T path, bool value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) boolean value at the defined node if defined node's ETag matched the ETag value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which Boolean data will be set.
   * @param value Boolean value to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<bool>() to get the boolean value that stored on the defined node.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].to<bool>() to get the current boolean value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setBool(FirebaseData *fbdo, T1 path, bool value, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setBoolAsync(FirebaseData *fbdo, T1 path, bool value, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) boolean value and the virtual child ".priority" if defined ETag matches at the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setBool(FirebaseData *fbdo, T1 path, bool value, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setBoolAsync(FirebaseData *fbdo, T1 path, bool value, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) string at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which string data will be set.
   * @param value String or text to set.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<String>() to get the string value that stored on the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setString(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setStringAsync(FirebaseData *fbdo, T1 path, T2 value)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) string value and virtual child ".priority" at the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setString(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setStringAsync(FirebaseData *fbdo, T1 path, T2 value, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) string at the defined node if defined node's ETag matched the ETag value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which string data will be set.
   * @param value String or text to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   * Call [FirebaseData object].to<String>() to get the string value that stored on the defined node.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also, call [FirebaseData object].to<String>() to get the current string value.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setString(FirebaseData *fbdo, T1 path, T2 value, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setStringAsync(FirebaseData *fbdo, T1 path, T2 value, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set string data and the virtual child ".priority" if defined ETag matches at the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setString(FirebaseData *fbdo, T1 path, T2 value, float &priority, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setStringAsync(FirebaseData *fbdo, T1 path, T2 value, float &priority, T3 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the child (s) nodes to the defined node.
   * The old content in defined node will be replaced.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which child (s) nodes will be replaced or set.
   * @param json The pointer to FirebaseJson object.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   *
   * Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr to get the JSON data that stored on the defined node.
   */
  template <typename T = const char *>
  bool setJSON(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(toStringPtr(_NO_PAYLOAD)),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setJSONAsync(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(toStringPtr(_NO_PAYLOAD)),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the child (s) nodes and virtual child ".priority" at the defined node.
   */
  template <typename T = const char *>
  bool setJSON(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setJSONAsync(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the child (s) nodes to the defined node, if defined node's ETag matched the ETag value.
   * The old content in defined node will be replaced.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which child(s) nodes will be replaced or set.
   * @param json The pointer to FirebaseJson object.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.
   *
   * Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr to get the JSON data that stored on the defined node.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr to get the JSON data.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSON(FirebaseData *fbdo, T1 path, FirebaseJson *json, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSONAsync(FirebaseData *fbdo, T1 path, FirebaseJson *json, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the child (s) nodes and the virtual child ".priority" if defined ETag matches at the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSON(FirebaseData *fbdo, T1 path, FirebaseJson *json, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSONAsync(FirebaseData *fbdo, T1 path, FirebaseJson *json, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the array to the defined node.
   * The old content in defined node will be replaced.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which array will be replaced or set.
   * @param arr The pointer to FirebaseJsonArray object.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data that successfully stores in the database.
   *
   * Call [FirebaseData object].to<FirebaseJsonArray>() and [FirebaseData object].to<FirebaseJsonArray*>() will return reference to object and
   * pointer to FirebaseJsonArray object that contains the array from payload.
   */
  template <typename T = const char *>
  bool setArray(FirebaseData *fbdo, T path, FirebaseJsonArray *arr)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setArrayAsync(FirebaseData *fbdo, T path, FirebaseJsonArray *arr)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) array and virtual child ".priority" at the defined node.
   */
  template <typename T = const char *>
  bool setArray(FirebaseData *fbdo, T path, FirebaseJsonArray *arr, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setArrayAsync(FirebaseData *fbdo, T path, FirebaseJsonArray *arr, float &priority)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the array to the defined node if defined node's ETag matched the ETag value.
   * The old content in defined node will be replaced.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which array will be replaced or set.
   * @param arr The pointer to FirebaseJsonArray object.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   *
   * Also call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * And[FirebaseData object].to<FirebaseJsonArray>() and [FirebaseData object].to<FirebaseJsonArray*>() will return reference to object and
   * pointer to FirebaseJsonArray object that contains the array from payload.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setArray(FirebaseData *fbdo, T1 path, FirebaseJsonArray *arr, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setArrayAsync(FirebaseData *fbdo, T1 path, FirebaseJsonArray *arr, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) array and the virtual child ".priority" if defined ETag matches at the defined node.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setArray(FirebaseData *fbdo, T1 path, FirebaseJsonArray *arr, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setArrayAsync(FirebaseData *fbdo, T1 path, FirebaseJsonArray *arr, float &priority, T2 ETag)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, toAddr(priority), toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Set (put) the blob (binary data) at the defined node.
   * The old content in defined node will be replaced.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to node in which binary data will be set.
   * @param blob Byte array of data.
   * @param size Size of the byte array.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note No payload returned from the server.
   */
  template <typename T = const char *>
  bool setBlob(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, toAddr(*blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, size, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setBlobAsync(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, toAddr(*blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, size, toStringPtr(_NO_FILE));
  }

  /** Set blob (binary data) at the defined node if defined node's ETag matched the ETag value.
   * The old content in defined node will be replaced.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which binary data will be set.
   * @param blob Byte array of data.
   * @param size Size of the byte array.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note No payload returned from the server.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setBlob(FirebaseData *fbdo, T1 path, uint8_t *blob, size_t size, T2 ETag)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, toAddr(*blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, size, toStringPtr(_NO_FILE));
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setBlobAsync(FirebaseData *fbdo, T1 path, uint8_t *blob, size_t size, T2 ETag)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, toAddr(*blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, size, toStringPtr(_NO_FILE));
  }

  /** Set (put) the binary data from file to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param path The path to the node in which binary data will be set.
   * @param fileName  The file path includes its name.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note No payload returned from the server.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName,
               RTDB_UploadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_file, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(fileName), storageType, NULL, callback);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path,
                    T2 fileName, RTDB_UploadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_file, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(fileName), storageType, NULL, callback);
  }

  /** Set (put) the binary data from file to the defined node if defined node's ETag matched the ETag value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param path The path to the node in which binary data from the file will be set.
   * @param fileName  The file path includes its name.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note No payload returned from the server.
   *
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName,
               T3 ETag, RTDB_UploadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_file, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(fileName), storageType, NULL, callback);
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path,
                    T2 fileName, T3 ETag, RTDB_UploadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, rtdb_set_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_file, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(fileName), storageType, NULL, callback);
  }

  /** Set (put) the Firebase server's timestamp to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which timestamp will be set.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].to<int>() will return the integer value of timestamp in seconds
   * or [FirebaseData object].to<double>() to get millisecond timestamp.
   *
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place,
   * use printf("%.0lf\n", firebaseData.to<double>());.
   */
  template <typename T = const char *>
  bool setTimestamp(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path),
                        toStringPtr(pgm2Str(fb_esp_rtdb_pgm_str_39 /* "{\".sv\": \"timestamp\"}" */)),
                        d_timestamp, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool setTimestampAsync(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_put, toStringPtr(path),
                        toStringPtr(pgm2Str(fb_esp_rtdb_pgm_str_39 /* "{\".sv\": \"timestamp\"}" */)),
                        d_timestamp, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Update (patch) the child (s) nodes to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which child (s) nodes will be updated.
   * @param json The pointer to FirebaseJson object used for the update.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr
   * to get the JSON data that already updated on the defined node.
   */
  template <typename T = const char *>
  bool updateNode(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, http_patch, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool updateNodeAsync(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, http_patch, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Update (patch) the child (s) nodess and virtual child ".priority" to the defined node.
   */
  template <typename T = const char *>
  bool updateNode(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, http_patch, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool updateNodeAsync(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, http_patch, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Update (patch) the child (s) nodes to the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which child (s) nodes will be updated.
   * @param json The pointer to FirebaseJson object used for the update.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Owing to the objective of this function to reduce network data usage,
   * no payload will be returned from the server.
   */
  template <typename T = const char *>
  bool updateNodeSilent(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, rtdb_update_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool updateNodeSilentAsync(FirebaseData *fbdo, T path, FirebaseJson *json)
  {
    return buildRequest(fbdo, rtdb_update_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Update (patch) the child (s) nodes and virtual child ".priority" to the defined node.
   */
  template <typename T = const char *>
  bool updateNodeSilent(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, rtdb_update_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T = const char *>
  bool updateNodeSilentAsync(FirebaseData *fbdo, T path, FirebaseJson *json, float &priority)
  {
    return buildRequest(fbdo, rtdb_update_nocontent, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, getAddr(json), _NO_QUERY, toAddr(priority), toStringPtr(_NO_ETAG),
                        _IS_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read generic type of value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].to<int>(), [FirebaseData object].to<float>, [FirebaseData object].to<double>,
   * [FirebaseData object].to<bool>, [FirebaseData object].to<String>, [FirebaseData object].to<FirebaseJson>(),
   * [FirebaseData object].to<FirebaseJson*>(), [FirebaseData object].to<FirebaseJsonArray>(),
   * [FirebaseData object].to<FirebaseJsonArray*>(), [FirebaseData object].to<std::vector<uint8_t> *> and [FirebaseData object].to<File>()
   * corresponded to its type that get from [FirebaseData object].dataType.
   */
  template <typename T = const char *>
  bool get(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_any, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the integer value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.

    @return Boolean value, indicates the success of the operation.

    Call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data successfully stores in the database.

    Call [FirebaseData object].to<int> will return the integer value of
    payload returned from server.

    If the type of payload returned from server is not integer, float and double,
    the function [FirebaseData object].to<int>() will return zero (0).

  */
  template <typename T = const char *>
  bool getInt(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_integer, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the integer value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to int type variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an integer, float and double,
   * the target variable's value will be zero (0).
   */
  template <typename T1 = const char *, typename T2>
  bool getInt(FirebaseData *fbdo, T1 path, T2 target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_integer, getSubType(target), toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the float value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<float> will return the float value of payload returned from server.
   *
   * If the payload returned from server is not integer, float and double,
   * the function [FirebaseData object].to<float>() will return zero (0).
   */
  template <typename T = const char *>
  bool getFloat(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_float, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the float value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to float type variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an integer, float and double,
   * the target variable's value will be zero (0).
   */
  template <typename T1 = const char *, typename T2>
  bool getFloat(FirebaseData *fbdo, T1 path, T2 target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_float, _NO_SUB_TYPE, toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the double value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<double> will return the double value of payload returned from server.
   *
   * If the payload returned from server is not integer, float and double,
   * the function [FirebaseData object].to<double>() will return zero (0).
   *
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place,
   * use printf("%.9lf\n", firebaseData.to<double>()); for print value up to 9 decimal places.
   */
  template <typename T = const char *>
  bool getDouble(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_double, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the double value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to double type variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an integer, float and double,
   * the target variable's value will be zero (0).
   */
  template <typename T1 = const char *, typename T2>
  bool getDouble(FirebaseData *fbdo, T1 path, T2 target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_double, _NO_SUB_TYPE, toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the Boolean value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<bool> will return the boolean value of payload returned from server.
   *
   * If the type of payload returned from the server is not Boolean,
   * the function [FirebaseData object].to<bool>() will return false.
   */
  template <typename T = const char *>
  bool getBool(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_boolean, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the Boolean value at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to boolean type variable to store the value.
   *  @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not Boolean,
   * the target variable's value will be false.
   */
  template <typename T1 = const char *, typename T2>
  bool getBool(FirebaseData *fbdo, T1 path, T2 target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_boolean, _NO_SUB_TYPE, toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the string at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<String> will return the String value of payload returned from server.
   *
   * If the type of payload returned from the server is not a string,
   * the function [FirebaseData object].to<String>() will return empty string.
   */
  template <typename T = const char *>
  bool getString(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the string at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to String, std::string or chars array variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the target is chars array, the size of chars array should be greater than
   * the size of payload string to prevent error.
   */
  template <typename T1 = const char *, typename T2>
  bool getString(FirebaseData *fbdo, T1 path, T2 target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_string, getSubType(target), toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the child (s) nodes at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<FirebaseJson>() and [FirebaseData object].to<FirebaseJson *>() will return reference to object and pointer to FirebaseJson object from payload.
   *
   * If the type of payload returned from server is not json,
   * the function [FirebaseData object].to<FirebaseJson>() will contain empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the JSON string at the defined node.
   * The returned the pointer to FirebaseJson that contains JSON payload represents the child nodes and their value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to FirebaseJson object variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not JSON,
   * the target FirebaseJson object will contain an empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData *fbdo, T path, FirebaseJson *target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the JSON string at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param query QueryFilter class to set query parameters to filter data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The Available query parameters for filtering the data are the following.
   *
   * QueryFilter.orderBy       Required parameter to specify which data used for data filtering included child key, key, and value.
   *                           Use "$key" for filtering data by keys of all nodes at the defined node.
   *                           Use "$value" for filtering data by value of all nodes at the defined node.
   *                           Use "$priority" for filtering data by "virtual child" named .priority of all nodes.
   *                           Use any child key to filter by that key.
   *
   * QueryFilter.limitToFirst  The total children (number) to filter from the first child.
   * QueryFilter.limitToLast   The total last children (number) to filter.
   * QueryFilter.startAt       Starting value of range (number or string) of query upon orderBy param.
   * QueryFilter.endAt         Ending value of range (number or string) of query upon orderBy param.
   * QueryFilter.equalTo       Value (number or string) matches the orderBy param
   *
   * Call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<FirebaseJson>() and [FirebaseData object].to<FirebaseJson *>() will return reference to object and pointer to FirebaseJson object from payload.
   *
   * If the type of payload returned from server is not JSON,
   * the function [FirebaseData object].to<FirebaseJson>() will contain empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData *fbdo, T path, QueryFilter *query)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, _NO_REF, getAddr(query), _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the JSON string at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to FirebaseJson object variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * If the type of payload returned from the server is not JSON,
   * the target FirebaseJson object will contain an empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData *fbdo, T path, QueryFilter *query, FirebaseJson *target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_json, _NO_SUB_TYPE, toAddr(*target), getAddr(query), _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the array at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<FirebaseJsonArray>() and [FirebaseData object].to<FirebaseJsonArray*>() will return reference to object and
   * pointer to FirebaseJsonArray object that contains the array from payload.
   *
   * If the type of payload returned from the server is not an array,
   * the array element in [FirebaseData object].to<FirebaseJsonArray>() will be empty.
   */
  template <typename T = const char *>
  bool getArray(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the array at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to FirebaseJsonArray object variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an array,
   * the target FirebaseJsonArray object will contain an empty array.
   */
  template <typename T = const char *>
  bool getArray(FirebaseData *fbdo, T path, FirebaseJsonArray *target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the array data at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param query QueryFilter class to set query parameters to filter data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The Available query parameters for filtering the data are the following.
   *
   * QueryFilter.orderBy       Required parameter to specify which data used for data filtering included child key, key, and value.
   *                           Use "$key" for filtering data by keys of all nodes at the defined node.
   *                           Use "$value" for filtering data by value of all nodes at the defined node.
   *                           Use "$priority" for filtering data by "virtual child" named .priority of all nodes.
   *                           Use any child key to filter by that key.
   *
   * QueryFilter.limitToFirst  The total children (number) to filter from the first child.
   * QueryFilter.limitToLast   The total last children (number) to filter.
   * QueryFilter.startAt       Starting value of range (number or string) of query upon orderBy param.
   * QueryFilter.endAt         Ending value of range (number or string) of query upon orderBy param.
   * QueryFilter.equalTo       Value (number or string) matches the orderBy param
   *
   * Call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<FirebaseJsonArray>() and [FirebaseData object].to<FirebaseJsonArray*>() will return reference to object and
   * pointer to FirebaseJsonArray object that contains the array from payload.
   *
   * If the type of payload returned from the server is not an array,
   * the function [FirebaseData object].to<FirebaseJsonArray>() will contain empty array.
   */
  template <typename T = const char *>
  bool getArray(FirebaseData *fbdo, T path, QueryFilter *query)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, _NO_REF, getAddr(query), _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the array data at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to FirebaseJsonArray object variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an array,
   * the target FirebaseJsonArray object will contain an empty array.
   */
  template <typename T = const char *>
  bool getArray(FirebaseData *fbdo, T path, QueryFilter *query, FirebaseJsonArray *target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_array, _NO_SUB_TYPE, toAddr(*target), getAddr(query), _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the blob (binary data) at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType or [FirebaseData object].dataTypeNum to determine what type of data
   * successfully stores in the database.
   *
   * Call [FirebaseData object].to<std::vector<uint8_t> *> will return the pointer to uint8_t dynamic array data of payload returned from server.
   *
   * If the type of payload returned from the server is not a blob,
   * the function [FirebaseData object].blobData will return empty array.
   */
  template <typename T = const char *>
  bool getBlob(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Read (get) the blob (binary data) at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node
   * @param target The pointer to uint8_t vector variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not a blob,
   * the target variable value will be an empty array.
   */
  template <typename T = const char *>
  bool getBlob(FirebaseData *fbdo, T path, MB_VECTOR<uint8_t> *target)
  {
    return buildRequest(fbdo, http_get, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_blob, _NO_SUB_TYPE, toAddr(*target), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Download file data at the defined node and save to storage memory.
   *
   * The downloaded data will be decoded to binary and save to SD card/Flash memory,
   * then please make sure that data at the defined node is the file type.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param nodePath The path to the node that file data will be downloaded.
   * @param fileName  The file path includes its name.
   * @param callback Optional. The callback function that accept RTDB_DownloadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool getFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 nodePath,
               T2 fileName, RTDB_DownloadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, http_get, toStringPtr(nodePath), toStringPtr(_NO_PAYLOAD),
                        d_file, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(fileName), storageType, callback);
  }

  /** Download a firmware file from the database.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param fwPath  The firmware data path.
   * @param callback Optional. The callback function that accept RTDB_DownloadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note: In ESP8266, this function will allocate 16k+ memory for internal SSL client.
   *
   * Firmware data is the bin file that stored on datanbase using pushFile or setFile function.
   */
  template <typename T = const char *>
  bool downloadOTA(FirebaseData *fbdo, T fwPath, RTDB_DownloadProgressCallback callback = NULL)
  {
    return buildRequest(fbdo, http_get, toStringPtr(fwPath), toStringPtr(_NO_PAYLOAD),
                        d_file_ota, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE), mem_storage_type_undefined, callback);
  }

  /** Delete all child nodes at the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node to be deleted.
   * @return Boolean value, indicates the success of the operation.
   */
  template <typename T = const char *>
  bool deleteNode(FirebaseData *fbdo, T path)
  {
    return buildRequest(fbdo, http_delete, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Delete all child nodes at the defined node if defined node's ETag matched the ETag value.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node to be deleted.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool deleteNode(FirebaseData *fbdo, T1 path, T2 ETag)
  {
    return buildRequest(fbdo, http_delete, toStringPtr(path), toStringPtr(_NO_PAYLOAD),
                        d_string, _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(ETag),
                        _NO_ASYNC, _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  /** Delete nodes that its timestamp node exceeded the data retaining period.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The parent path of children nodes that is being deleted.
   * @param timestampNode The sub-child node that keep the timestamp.
   * @param limit The maximum number of children nodes to delete at once, 30 is maximum.
   * @param dataRetentionPeriod The period in seconds of data in the past which will be retained.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type
   * is Email/Password sign-in.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = size_t, typename T4 = unsigned long>
  bool deleteNodesByTimestamp(FirebaseData *fbdo, T1 path, T2 timestampNode, T3 limit, T4 dataRetentionPeriod)
  {
    return mDeleteNodesByTimestamp(fbdo, toStringPtr(path), toStringPtr(timestampNode),
                                   toStringPtr(limit, -1), toStringPtr(dataRetentionPeriod, -1));
  }

  /** Subscribe to the value changes on the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node to subscribe.
   * @return Boolean value, indicates the success of the operation.
   */
  template <typename T = const char *>
  bool beginStream(FirebaseData *fbdo, T path) { return mBeginStream(fbdo, toStringPtr(path)); }

  /** Subscribe to the value changes on the children of the defined node.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param parentPath The path to the parent node to subscribe.
   * @return Boolean value, indicates the success of the operation.
   */
  template <typename T = const char *>
  bool beginMultiPathStream(FirebaseData *fbdo, T parentPath) { return mBeginMultiPathStream(fbdo, toStringPtr(parentPath)); }

  /** Read the stream event data at the defined node.
   *
   * Once beginStream was called e.g. in setup(), the readStream function
   * should call inside the continuous loop block.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Using the shared Firebase Data object for stream read/monitoring associated
   * with normal Firebase call e.g. read, set, push, update and delete will break or interrupt
   * the current stream connection.
   *
   * he stream will be resumed or reconnected automatically when calling the function readStream.
   */
  bool readStream(FirebaseData *fbdo);

  /** End the stream connection at a defined node.
   *
   * It can be restart again by calling the function beginStream.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @return Boolean value, indicates the success of the operation.
   */
  bool endStream(FirebaseData *fbdo);

  /** Set the stream callback functions.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param dataAvailableCallback The Callback function that accepts StreamData parameter.
   * @param timeoutCallback The Callback function will be called when the stream connection was timed out (optional).
   *
   * ESP32 only parameter
   * @param streamTaskStackSize The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).
   *
   * @note The dataAvailableCallback will be called When data in the defined path changed or the stream path changed or stream connection
   * was resumed from getXXX, setXXX, pushXXX, updateNode, deleteNode.
   *
   * The payload returned from the server will be one of these integer, float, string, JSON and blob types.
   *
   * Call [StreamData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [StreamData object].xxxData will return the appropriate data type of
   * the payload returned from the server.
   */

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
  void setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback,
                         FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize = 8192);
#else
  void setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback,
                         FirebaseData::StreamTimeoutCallback timeoutCallback);
#endif

  /** Set the multiple paths stream callback functions.
   * setMultiPathStreamCallback should be called before Firebase.beginMultiPathStream.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param multiPathDataCallback The Callback function that accepts MultiPathStreamData parameter.
   * @param timeoutCallback The Callback function will be called when the stream connection was timed out (optional).
   *
   * ESP32 only parameter
   * @param streamTaskStackSize The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).
   *
   * @note The multiPathDataCallback will be called When children value of the defined node changed or the stream path changed or stream connection
   * was resumed from normal Firebase calls.
   *
   * The payload returned from the server will be one of these types e.g. boolean, integer, float, string, JSON, array, blob and file.
   *
   * Call [MultiPathStreamData object].get to get the child node value, type and its data path.
   *
   * The properties [MultiPathStreamData object].value, [MultiPathStreamData object].dataPath, and [MultiPathStreamData object].type will return the value, path of data, and type of data respectively.
   *
   * These properties will store the result from calling the function [MultiPathStreamData object].get.
   */

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
  void setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback,
                                  FirebaseData::StreamTimeoutCallback timeoutCallback = NULL, size_t streamTaskStackSize = 8192);
#else
  void setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback,
                                  FirebaseData::StreamTimeoutCallback timeoutCallback = NULL);
#endif

  /** Remove stream callback functions.
   *
   * @param fbdo The pointer to Firebase Data Object.
   */
  void removeStreamCallback(FirebaseData *fbdo);

  /** Remove multiple paths stream callback functions.
   *
   * @param fbdo The pointer to Firebase Data Object.
   */
  void removeMultiPathStreamCallback(FirebaseData *fbdo);

  /** Run stream manually.
   * To manually triggering the stream callback function, this should call repeatedly in loop().
   */
  void runStream()
  {
    mStopStreamLoopTask();
    mRunStream();
  }

  /** Backup (download) the database at the defined node to the storage memory.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param nodePath The path to the node to be backuped.
   * @param fileName File name to save.
   * @param callback Optional. The callback function that accept RTDB_DownloadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Only 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension) can be saved to SD card/Flash memory.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool backup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 nodePath, T2 fileName,
              RTDB_DownloadProgressCallback callback = NULL)
  {
    return mBackup(fbdo, storageType, toStringPtr(nodePath), toStringPtr(fileName), callback);
  }

  /** Restore the database at a defined path using backup file saved on SD card/Flash memory.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param nodePath The path to the node to be restored the data.
   * @param fileName File name to read.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool restore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 nodePath, T2 fileName,
               RTDB_UploadProgressCallback callback = NULL)
  {
    return mRestore(fbdo, storageType, toStringPtr(nodePath), toStringPtr(fileName), callback);
  }

  /** Set maximum Firebase read/store retry operation (0 - 255)
   * in case of network problems and buffer overflow.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param num The maximum retry.
   */
  void setMaxRetry(FirebaseData *fbdo, uint8_t num);

#if defined(ENABLE_ERROR_QUEUE)

  /** Set the maximum Firebase Error Queues in the collection (0 255).
   *
   * Firebase read/store operation causes by network problems and buffer overflow
   * will be added to Firebase Error Queues collection.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param num The maximum Firebase Error Queues.
   */
  void setMaxErrorQueue(FirebaseData *fbdo, uint8_t num);

  /** Save Firebase Error Queues as file in flash memory (save only database store queues).
   *
   * The Firebase read (get) operation will not save.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param filename Filename to be saved.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   */
  template <typename T = const char *>
  bool saveErrorQueue(FirebaseData *fbdo, T filename, fb_esp_mem_storage_type storageType)
  {
    return mSaveErrorQueue(fbdo, toStringPtr(filename), storageType);
  }

  /** Delete file in storage memory.
   *
   * @param filename File name to delete.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   */
  template <typename T = const char *>
  bool deleteStorageFile(T filename, fb_esp_mem_storage_type storageType)
  {
    return mDeleteStorageFile(toStringPtr(filename), storageType);
  }

  /** Restore the Firebase Error Queues from the queue file (flash memory).
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param filename Filename to be read and restore queues.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   */
  template <typename T = const char *>
  bool restoreErrorQueue(FirebaseData *fbdo, T filename, fb_esp_mem_storage_type storageType)
  {
    return mRestoreErrorQueue(fbdo, toStringPtr(filename), storageType);
  }

  /** Determine the number of Firebase Error Queues stored in a defined file (flash memory).
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param filename Filename to be read and count for queues.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @return Number (0-255) of queues store in defined queue file.
   */
  template <typename T = const char *>
  uint8_t errorQueueCount(FirebaseData *fbdo, T filename, fb_esp_mem_storage_type storageType)
  {
    return mErrorQueueCount(fbdo, toStringPtr(filename), storageType);
  }

  /** Determine number of queues in Firebase Data object's Error Queues collection.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @return Number (0-255) of queues in Firebase Data object's error queue collection.
   */
  uint8_t errorQueueCount(FirebaseData *fbdo);

  /** Determine whether the Firebase Error Queues collection was full or not.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @return Boolean value, indicates the full of queue.
   */
  bool isErrorQueueFull(FirebaseData *fbdo);

  /** Process all failed Firebase operation queue items when the network is available.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param callback a Callback function that accepts QueueInfo parameter.
   */
  void processErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback = NULL);

  /** Return Firebase Error Queue ID of last Firebase Error.
   *
   * Return 0 if there is no Firebase Error from the last operation.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @return Number of Queue ID.
   */
  uint32_t getErrorQueueID(FirebaseData *fbdo);

  /** Determine whether the Firebase Error Queue currently exists in the Error Queue collection or not.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param errorQueueID The Firebase Error Queue ID get from getErrorQueueID.
   * @return Boolean type status indicates the queue existence.
   */
  bool isErrorQueueExisted(FirebaseData *fbdo, uint32_t errorQueueID);

  /** Start the Firebase Error Queues Auto Run Process.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param callback The Callback function that accepts QueueInfo Object as a parameter, optional.
   *
   *
   * ESP32 only parameter
   * @param queueTaskStackSize The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).
   *
   * @note The following functions are available from QueueInfo Object accepted by the callback.
   *
   * queueInfo.totalQueues(), get the total Error Queues in Error Queue Collection.
   *
   * queueInfo.currentQueueID(), get current Error Queue ID that is being processed.
   *
   * queueInfo.isQueueFull(), determine whether Error Queue Collection is full or not.
   *
   * queueInfo.dataType(), get a string of the Firebase call data type that is being processed of current Error Queue.
   *
   * queueInfo.method(), get a string of the Firebase call method that is being processed of current Error Queue.
   *
   * queueInfo.path(), get a string of the Firebase call path that is being processed of current Error Queue.
   */
#if defined(ESP32) || defined(MB_ARDUINO_PICO) || defined(ESP8266)
#if defined(ESP32) || defined(MB_ARDUINO_PICO)
  void beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback = NULL,
                              size_t queueTaskStackSize = 8192);
#else
  void beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback = NULL);
#endif
#endif

  /** Stop the Firebase Error Queues Auto Run Process.
   *
   * @param fbdo The pointer to Firebase Data Object.
   */
  void endAutoRunErrorQueue(FirebaseData *fbdo);

  /** Clear all Firbase Error Queues in Error Queue collection.
   *
   * @param fbdo The pointer to Firebase Data Object.
   */
  void clearErrorQueue(FirebaseData *fbdo);

#endif

  template <typename T1 = const char *, typename T2>
  bool push(FirebaseData *fbdo, T1 path, T2 value)
  {
    return dataPushHandler(fbdo, path, value, _NO_PRIORITY, false);
  }

  template <typename T1 = const char *, typename T2>
  bool push(FirebaseData *fbdo, T1 path, T2 value, float priority)
  {
    return dataPushHandler(fbdo, path, value, priority, false);
  }

  template <typename T1 = const char *, typename T2>
  bool pushAsync(FirebaseData *fbdo, T1 path, T2 value)
  {
    return dataPushHandler(fbdo, path, value, _NO_PRIORITY, true);
  }

  template <typename T1 = const char *, typename T2>
  bool pushAsync(FirebaseData *fbdo, T1 path, T2 value, float priority)
  {
    return dataPushHandler(fbdo, path, value, priority, true);
  }

  template <typename T = const char *>
  bool push(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return dataPushHandler(fbdo, path, blob, size, false);
  }

  template <typename T = const char *>
  bool pushAsync(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return dataPushHandler(fbdo, path, blob, size, true);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool push(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName)
  {
    return dataPushHandler(fbdo, path, fileName, storageType, false);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName)
  {
    return dataPushHandler(fbdo, path, fileName, storageType, true);
  }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData *fbdo, T1 path, T2 value)
  {
    return dataSetHandler(fbdo, path, value, _NO_PRIORITY, toStringPtr(_NO_ETAG), false);
  }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData *fbdo, T1 path, T2 value)
  {
    return dataSetHandler(fbdo, path, value, _NO_PRIORITY, toStringPtr(_NO_ETAG), true);
  }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData *fbdo, T1 path, T2 value, float priority)
  {
    return dataSetHandler(fbdo, path, value, priority, toStringPtr(_NO_ETAG), false);
  }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData *fbdo, T1 path, T2 value, float priority)
  {
    return dataSetHandler(fbdo, path, value, priority, toStringPtr(_NO_ETAG), true);
  }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool set(FirebaseData *fbdo, T1 path, T2 value, T3 etag)
  {
    return dataSetHandler(fbdo, path, value, _NO_PRIORITY, etag, false);
  }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool setAsync(FirebaseData *fbdo, T1 path, T2 value, T3 etag)
  {
    return dataSetHandler(fbdo, path, value, _NO_PRIORITY, etag, true);
  }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool set(FirebaseData *fbdo, T1 path, T2 value, float priority, T3 etag)
  {
    return dataSetHandler(fbdo, path, value, priority, etag, false);
  }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool setAsync(FirebaseData *fbdo, T1 path, T2 value, float priority, T3 etag)
  {
    return dataSetHandler(fbdo, path, value, priority, etag, true);
  }

  template <typename T = const char *>
  bool set(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return dataSetHandler(fbdo, path, blob, size, _NO_PRIORITY, false);
  }

  template <typename T = const char *>
  bool setAsync(FirebaseData *fbdo, T path, uint8_t *blob, size_t size)
  {
    return dataSetHandler(fbdo, path, blob, size, _NO_PRIORITY, true);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool set(FirebaseData *fbdo, T1 path, uint8_t *blob, size_t size, T2 etag)
  {
    return dataSetHandler(fbdo, path, blob, size, etag, false);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setAsync(FirebaseData *fbdo, T1 path, uint8_t *blob, size_t size, T2 etag)
  {
    return dataSetHandler(fbdo, path, blob, size, etag, true);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName)
  {
    return dataSetHandler(fbdo, path, fileName, storageType, _NO_PRIORITY, false);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName)
  {
    return dataSetHandler(fbdo, path, fileName, storageType, _NO_PRIORITY, true);
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName, T3 etag)
  {
    return dataSetHandler(fbdo, path, fileName, storageType, etag, false);
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, T1 path, T2 fileName, T3 etag)
  {
    return dataSetHandler(fbdo, path, fileName, storageType, etag, true);
  }

private:
  void rescon(FirebaseData *fbdo, const char *host, fb_esp_rtdb_request_info_t *req);
  void clearDataStatus(FirebaseData *fbdo);
  bool handleRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  bool sendRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  int preRequestCheck(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  fb_esp_request_method getHTTPMethod(fb_esp_rtdb_request_info_t *req);
  bool hasPayload(struct fb_esp_rtdb_request_info_t *req);
  bool sendRequestHeader(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  int getPayloadLen(fb_esp_rtdb_request_info_t *req);
  bool waitResponse(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req);
  bool handleResponse(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req);
  int openFile(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, mb_fs_open_mode mode, bool closeSession = false);
  void waitRxReady(FirebaseData *fbdo, unsigned long &dataTime);
  void parsePayload(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, struct server_response_data_t &response,
                    MB_String payload);
  void handlePayload(FirebaseData *fbdo, struct server_response_data_t &response, const MB_String &payload);
  bool processRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  bool encodeFileToClient(FirebaseData *fbdo, size_t bufSize, const MB_String &filePath,
                          fb_esp_mem_storage_type storageType, struct fb_esp_rtdb_request_info_t *req);
  void setPtrValue(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  bool buildRequest(FirebaseData *fbdo, fb_esp_request_method method, MB_StringPtr path, MB_StringPtr payload,
                    fb_esp_data_type type, int subtype, uint32_t value_addr, uint32_t query_addr, uint32_t priority_addr,
                    MB_StringPtr etag, bool async, bool queue, size_t blob_size, MB_StringPtr filename,
                    fb_esp_mem_storage_type storage_type = mem_storage_type_undefined,
                    RTDB_DownloadProgressCallback downloadCallback = NULL, RTDB_UploadProgressCallback uploadCallback = NULL);
  bool mSetRules(FirebaseData *fbdo, MB_StringPtr rules, fb_esp_mem_storage_type storageType,
                 MB_StringPtr filename, RTDB_UploadProgressCallback callback = NULL);
  bool mSetReadWriteRules(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr var,
                          MB_StringPtr readVal, MB_StringPtr writeVal, MB_StringPtr databaseSecret);
  bool mPathExisted(FirebaseData *fbdo, MB_StringPtr path);
  String mGetETag(FirebaseData *fbdo, MB_StringPtr path);
  bool mGetShallowData(FirebaseData *fbdo, MB_StringPtr path);
  bool mDeleteNodesByTimestamp(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr timestampNode,
                               MB_StringPtr limit, MB_StringPtr dataRetentionPeriod);
  bool mBeginMultiPathStream(FirebaseData *fbdo, MB_StringPtr parentPath);
  bool mBackup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr nodePath,
               MB_StringPtr fileName, RTDB_DownloadProgressCallback callback = NULL);
  bool mRestore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr nodePath,
                MB_StringPtr fileName, RTDB_UploadProgressCallback callback = NULL);
  uint8_t mErrorQueueCount(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType);
  bool mRestoreErrorQueue(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType);
  bool mDeleteStorageFile(MB_StringPtr filename, fb_esp_mem_storage_type storageType);
  bool mSaveErrorQueue(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType);
  void setBlobRef(FirebaseData *fbdo, int addr);
  void mSetwriteSizeLimit(FirebaseData *fbdo, MB_StringPtr size);
  bool mGetRules(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr filename,
                 RTDB_DownloadProgressCallback callback = NULL);
  bool handleStreamRequest(FirebaseData *fbdo, const MB_String &path);
  bool connectionError(FirebaseData *fbdo);
  bool handleStreamRead(FirebaseData *fbdo);
  bool exitStream(FirebaseData *fbdo, bool status);
  void trimEndJson(MB_String &payload);
  void readBase64FileChunk(FirebaseData *fbdo, MB_String &payload, struct fb_esp_tcp_response_handler_t &tcpHandler,
                           struct server_response_data_t &response, int chunkSize, bool &streamDataComplete);
  void handleNoContent(FirebaseData *fbdo, struct server_response_data_t &response);
  bool parseTCPResponse(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req,
                        fb_esp_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response);
  bool handleDownload(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, struct fb_esp_tcp_response_handler_t &tcpHandler,
                      struct server_response_data_t &response);
  bool endDownloadOTA(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, struct fb_esp_tcp_response_handler_t &tcpHandler,
                      struct server_response_data_t &response);
  void endDownload(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, struct fb_esp_tcp_response_handler_t &tcpHandler,
                   struct server_response_data_t &response);
  int handleRedirect(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, struct fb_esp_tcp_response_handler_t &tcpHandler,
                     struct server_response_data_t &response);
  void sendCB(FirebaseData *fbdo);
  void splitStreamPayload(const MB_String &payloads, MB_VECTOR<MB_String> &payload);
  void parseStreamPayload(FirebaseData *fbdo, const MB_String &payload);
  void storeToken(MB_String &atok, const char *databaseSecret);
  void restoreToken(MB_String &atok, fb_esp_auth_token_type tk);
  bool mSetQueryIndex(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr node, MB_StringPtr databaseSecret);
  bool mBeginStream(FirebaseData *fbdo, MB_StringPtr path);
  void mSetReadTimeout(FirebaseData *fbdo, MB_StringPtr millisec);
  void reportUploadProgress(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req, size_t readBytes);
  void reportDownloadProgress(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req, size_t readBytes);
  void makeUploadStatus(RTDB_UploadStatusInfo &info, const MB_String &local, const MB_String &remote,
                        fb_esp_rtdb_upload_status status, size_t progress, size_t size, int elapsedTime, const MB_String &msg);
  void sendUploadCallback(FirebaseData *fbdo, RTDB_UploadStatusInfo &in, RTDB_UploadProgressCallback cb,
                          RTDB_UploadStatusInfo *out);
  void sendDownloadCallback(FirebaseData *fbdo, RTDB_DownloadStatusInfo &in, RTDB_DownloadProgressCallback cb,
                            RTDB_DownloadStatusInfo *out);
  void makeDownloadStatus(RTDB_DownloadStatusInfo &info, const MB_String &local, const MB_String &remote,
                          fb_esp_rtdb_download_status status, size_t progress, size_t size, int elapsedTime, const MB_String &msg);
#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
  void runStreamTask(FirebaseData *fbdo, const char *taskName);
#else
  void runStreamTask();
#endif
  void mStopStreamLoopTask();
  void mRunStream();

#if defined(ENABLE_ERROR_QUEUE)

  void addQueueData(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);

#if defined(ESP8266)
  void runErrorQueueTask();
#endif

  uint8_t openErrorQueue(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType, uint8_t mode);
#if defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)
  uint8_t readQueueFile(FirebaseData *fbdo, fs::File &file, QueueItem &item, uint8_t mode);
#endif
#if defined(MBFS_ESP32_SDFAT_ENABLED)
  uint8_t readQueueFileSdFat(FirebaseData *fbdo, MBFS_SD_FILE &file, QueueItem &item, uint8_t mode);
#endif

#endif

protected:
  int getPrec(bool dbl)
  {
    if (Signer.getCfg())
    {
      if (dbl)
        return Signer.getCfg()->internal.fb_double_digits;
      else
        return Signer.getCfg()->internal.fb_float_digits;
    }
    else
    {
      if (dbl)
        return 9;
      else
        return 5;
    }
  }

  uint32_t getAddr(QueryFilter *v) { return reinterpret_cast<uint32_t>(v); }
  uint32_t getAddr(FirebaseJson *v) { return reinterpret_cast<uint32_t>(v); }
  uint32_t getAddr(FirebaseJsonArray *v) { return reinterpret_cast<uint32_t>(v); }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, bool async) ->
      typename enable_if<is_string<T1>::value && is_num_int<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1), d_integer,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(_NO_ETAG), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, bool async) ->
      typename enable_if<is_string<T1>::value && is_bool<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1), d_boolean,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(_NO_ETAG), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, float>::value, bool>::type
  {
    return bbuildRequestEQ(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1), d_float,
                           _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(_NO_ETAG), async, _NO_QUEUE,
                           _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, double>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value, -1), d_double,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(_NO_ETAG), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, bool async) ->
      typename enable_if<is_string<T1>::value && is_string<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(value), d_string,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(_NO_ETAG), async,
                        _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 json, int priority_addr, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, FirebaseJson *>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_json,
                        _NO_SUB_TYPE, getAddr(json), _NO_QUERY, priority_addr, toStringPtr(_NO_ETAG), async,
                        _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 arr, int priority_addr, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, FirebaseJsonArray *>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_array,
                        _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, priority_addr, toStringPtr(_NO_ETAG), async,
                        _NO_QUEUE, _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 blob, size_t size, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, uint8_t *>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_blob,
                        _NO_SUB_TYPE, toAddr(blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG), async,
                        _NO_QUEUE, size, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataPushHandler(FirebaseData *fbdo, T1 path, T2 filename, fb_esp_mem_storage_type storageType, bool async) ->
      typename enable_if<is_string<T1>::value && is_string<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_post, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_file,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(_NO_ETAG), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(filename), storageType);
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_bool<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1), d_boolean,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_num_int<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1), d_integer,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, float>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1), d_float,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, double>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value, -1), d_double,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 value, int priority_addr, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_string<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(value), d_string,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, priority_addr, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 json, int priority_addr, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, FirebaseJson *>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_json,
                        _NO_SUB_TYPE, getAddr(json), _NO_QUERY, priority_addr, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 arr, int priority_addr, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_same<T2, FirebaseJsonArray *>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_array,
                        _NO_SUB_TYPE, getAddr(arr), _NO_QUERY, priority_addr, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, uint8_t *blob, size_t size, T2 etag, bool async) ->
      typename enable_if<is_string<T1>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_blob,
                        _NO_SUB_TYPE, toAddr(blob), _NO_QUERY, _NO_PRIORITY, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(_NO_FILE));
  }

  template <typename T1, typename T2, typename T3>
  auto dataSetHandler(FirebaseData *fbdo, T1 path, T2 filename, fb_esp_mem_storage_type storageType, T3 etag, bool async) ->
      typename enable_if<is_string<T1>::value && is_string<T2>::value, bool>::type
  {
    return buildRequest(fbdo, http_put, toStringPtr(path), toStringPtr(_NO_PAYLOAD), d_file,
                        _NO_SUB_TYPE, _NO_REF, _NO_QUERY, _NO_PRIORITY, toStringPtr(etag), async, _NO_QUEUE,
                        _NO_BLOB_SIZE, toStringPtr(filename), storageType);
  }
};

#endif

#endif // ENABLE