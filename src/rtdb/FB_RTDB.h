/**
 * Google's Firebase Realtime Database class, FB_RTDB.h version 1.0.9
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created April 4, 2021
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

#ifndef FIREBASE_RTDB_H
#define FIREBASE_RTDB_H
#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"
#include "QueueInfo.h"
#include "stream/FB_MP_Stream.h"
#include "stream/FB_Stream.h"

class FB_RTDB
{
  friend class Firebase_ESP_Client;

public:
  FB_RTDB();
  ~FB_RTDB();

  /** Stop Firebase and release all resources.
   * 
   * @param fbdo The pointer to Firebase Data Object.
  */
  void end(FirebaseData *fbdo);

#if defined(ESP32)
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
  void setReadTimeout(FirebaseData *fbdo, int millisec);

  /** Set the size limit of payload data that will write to the database for each request.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param size The size identified string e.g. tiny, small, medium, large and unlimited. 
   * 
   * @note Size string and its write timeout in seconds e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  */
  void setwriteSizeLimit(FirebaseData *fbdo, const char *size);

  /** Read the database rules.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @return Boolean value, indicates the success of the operation. 
   * 
   * @note Call [FirebaseData object].jsonData will return the JSON string value of 
   * database rules returned from the server.
  */
  bool getRules(FirebaseData *fbdo);

  /** Write the database rules.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param rules The JSON serialized string of the rules.
   * @return Boolean value, indicates the success of the operation.
  */
  bool setRules(FirebaseData *fbdo, const char *rules);

  /** Determine the existent of the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, true if the defined node was found.
   */
  bool pathExisted(FirebaseData *fbdo, const char *path);

  /** Determine the unique identifier (ETag) of current data at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return String of unique identifier.
  */
  String getETag(FirebaseData *fbdo, const char *path);

  /** Get the shallowed data at a defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation. 
   * 
   * @note Return the child data with the value or JSON object (the value will be truncated to true). 
   * Call [FirebaseData object].stringData() to get shallowed string (number, string and JSON object).
  */
  bool getShallowData(FirebaseData *fbdo, const char *path);

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
  bool setPriority(FirebaseData *fbdo, const char *path, float priority);

  /** Read the virtual child node ".priority" value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   */
  bool getPriority(FirebaseData *fbdo, const char *path);

  /** Append (post)  new integer value to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param intValue The appended value.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note The key or name of new created node will be stored in Firebase Data object, 
   * call [FirebaseData object].pushName() to get the key.
  */
  bool pushInt(FirebaseData *fbdo, const char *path, int intValue);

  bool push(FirebaseData *fbdo, const char *path, int intValue);

  /** Append (post) new integer value and the virtual child ".priority" to the defined node.
  */
  bool pushInt(FirebaseData *fbdo, const char *path, int intValue, float priority);

  bool push(FirebaseData *fbdo, const char *path, int intValue, float priority);

  /** Append (post) new float value to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float value will be appended.
   * @param floatValue The appended value.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note The key or name of new created node will be stored in Firebase Data object, 
   * call [FirebaseData object].pushName() to get the key.
  */
  bool pushFloat(FirebaseData *fbdo, const char *path, float floatValue);

  bool push(FirebaseData *fbdo, const char *path, float floatValue);

  /** Append (post) new float value and the virtual child ".priority" to the defined node.
  */
  bool pushFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority);

  bool push(FirebaseData *fbdo, const char *path, float floatValue, float priority);

  /** Append (post) new double value (8 bytes) to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float value will be appended.
   * @param doubleValue The appended value.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note The key or name of new created node will be stored in Firebase Data object, 
   * call [FirebaseData object].pushName() to get the key.
  */
  bool pushDouble(FirebaseData *fbdo, const char *path, double doubleValue);

  bool push(FirebaseData *fbdo, const char *path, double doubleValue);

  /** Append (post) new double value (8 bytes) and the virtual child ".priority" to the defined node.
  */
  bool pushDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority);

  bool push(FirebaseData *fbdo, const char *path, double doubleValue, float priority);

  /** Append (post) new Boolean value to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which Boolean value will be appended.
   * @param boolValue The appended value.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note The key or name of new created node will be stored in Firebase Data object, 
   * call [FirebaseData object].pushName() to get the key.

  */
  bool pushBool(FirebaseData *fbdo, const char *path, bool boolValue);

  bool push(FirebaseData *fbdo, const char *path, bool boolValue);

  /** Append (post) the new Boolean value and the virtual child ".priority" to the defined node.
  */
  bool pushBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority);

  bool push(FirebaseData *fbdo, const char *path, bool boolValue, float priority);

  /** Append (post) a new string (text) to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which string will be appended.
   * @param StringValue The appended value.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note The key or name of new created node will be stored in Firebase Data object, 
   * call [FirebaseData object].pushName() to get the key.
  */
  bool pushString(FirebaseData *fbdo, const char *path, const String &stringValue);

  bool push(FirebaseData *fbdo, const char *path, const char *stringValue);

  bool push(FirebaseData *fbdo, const char *path, const String &stringValue);

  /** Append (post) new string and the virtual child ".priority" to the defined node.
  */
  bool pushString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);

  bool push(FirebaseData *fbdo, const char *path, const char *stringValue, float priority);

  bool push(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);

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
  bool pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json);

  bool push(FirebaseData *fbdo, const char *path, FirebaseJson *json);

  /** Append (post) new child (s) and the virtual child ".priority" to the defined node.
  */
  bool pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

  bool push(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

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
  bool pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);

  bool push(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);

  /** Append (post) array and virtual child ".priority" at the defined node.
  */

  bool pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);

  bool push(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);

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
  bool pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);

  bool push(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);

  /** Append (post) new blob (binary data) and the virtual child ".priority" to the defined node.
  */
  bool pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);

  bool push(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);

  /** Append (post) new binary data from file stores on storage memory to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param path The path to the node in which binary data will be appended.
   * @param fileName The file path includes its name.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note The key or name of new created node will be stored in Firebase Data object, 
   * call [FirebaseData object].pushName() to get the key.
  */
  bool pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);

  bool push(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);

  /** Append (post) new binary data from file and the virtual child ".priority" to the defined node.
  */
  bool pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);

  bool push(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);

  /** Append (post) the new Firebase server's timestamp to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which timestamp will be appended.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note The key or name of new created node will be stored in Firebase Data object, 
   * call [FirebaseData object].pushName() to get the key.
   */
  bool pushTimestamp(FirebaseData *fbdo, const char *path);

  /** Set (put) the integer value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node which integer value will be set.
   * @param intValue Integer value to set.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].intData to get the integer value that stored on the defined node.
  */
  bool setInt(FirebaseData *fbdo, const char *path, int intValue);

  bool set(FirebaseData *fbdo, const char *path, int intValue);

  /** Set (put) the integer value and virtual child ".priority" at the defined node.
  */
  bool setInt(FirebaseData *fbdo, const char *path, int intValue, float priority);

  bool set(FirebaseData *fbdo, const char *path, int intValue, float priority);

  /** Set (put) the integer value at the defined node if defined node's ETag matched the defined ETag value.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which integer value will be set.
   * @param intValue Integer value to set.
   * @param ETag Known unique identifier string (ETag) of the defined node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].intData to get the integer value that stored on the defined node.
   * 
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 
   * 
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 
   * Also call [FirebaseData object].intData to get the current integer value.
  */
  bool setInt(FirebaseData *fbdo, const char *path, int intValue, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, int intValue, const char *ETag);

  /** Set (put) integer value and the virtual child ".priority" if defined ETag matches at the defined node 
  */
  bool setInt(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag);

  /** Set (put) float value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float value will be set.
   * @param floatValue Float value to set.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].floatData to get the float value that stored on the defined node.
  */
  bool setFloat(FirebaseData *fbdo, const char *path, float floatValue);

  bool set(FirebaseData *fbdo, const char *path, float floatValue);

  /** Set (put) float value and virtual child ".priority" at the defined node.
  */
  bool setFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority);

  bool set(FirebaseData *fbdo, const char *path, float floatValue, float priority);

  /** Set (put) float value at the defined node if defined node's ETag matched the ETag value.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float data will be set.
   * @param floatValue Float value to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].floatData to get the float value that stored on the defined node.
   * 
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 
   * 
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 
   * Also call [FirebaseData object].floatData to get the current float value.
   */
  bool setFloat(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag);

  /** Set (put) float value and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag);

  /** Set (put) double value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float data will be set.
   * @param doubleValue Double value to set.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].doubleData to get the double value that stored on the defined node.
   * 
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
   * use printf("%.9lf\n", firebaseData.doubleData()); for print the returned double value up to 9 decimal places.
  */
  bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue);

  bool set(FirebaseData *fbdo, const char *path, double doubleValue);

  /** Set (put) double value and virtual child ".priority" at the defined node.
  */
  bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority);

  bool set(FirebaseData *fbdo, const char *path, double doubleValue, float priority);

  /** Set (put) double value at the defined node if defined node's ETag matched the ETag value.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which float data will be set.
   * @param doubleValue Double value to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].doubleData to get the double value that stored on the defined node.
   * 
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 
   * 
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 
   * Also call [FirebaseData object].doubeData to get the current double value.
  */
  bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag);

  /** Set (put) double value and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag);

  /** Set (put) boolean value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which Boolean data will be set.
   * @param boolValue Boolean value to set.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].boolData to get the integer value that stored on the defined node.
  */
  bool setBool(FirebaseData *fbdo, const char *path, bool boolValue);

  bool set(FirebaseData *fbdo, const char *path, bool boolValue);

  /** Set (put) boolean value and virtual child ".priority" at the defined node.
  */
  bool setBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority);

  bool set(FirebaseData *fbdo, const char *path, bool boolValue, float priority);

  /** Set (put) boolean value at the defined node if defined node's ETag matched the ETag value.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which Boolean data will be set.
   * @param boolValue Boolean value to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].boolData to get the boolean value that stored on the defined node.
   * 
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 
   * 
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 
   * Also call [FirebaseData object].boolData to get the current boolean value.
  */
  bool setBool(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag);

  /** Set (put) boolean value and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag);

  /** Set (put) string at the defined node. 
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which string data will be set.
   * @param stringValue String or text to set.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].stringData to get the string value that stored on the defined node.
  */
  bool setString(FirebaseData *fbdo, const char *path, const String &stringValue);

  bool set(FirebaseData *fbdo, const char *path, const char *stringValue);

  bool set(FirebaseData *fbdo, const char *path, const String &stringValue);

  /** Set (put) string value and virtual child ".priority" at the defined node.
  */
  bool setString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);

  bool set(FirebaseData *fbdo, const char *path, const char *stringValue, float priority);

  bool set(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);

  /** Set (put) string at the defined node if defined node's ETag matched the ETag value.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which string data will be set.
   * @param stringValue String or text to set.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
   * Call [FirebaseData object].stringData to get the string value that stored on the defined node.
   * 
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   * 
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also, call [FirebaseData object].stringData to get the current string value.
   */
  bool setString(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, const char *stringValue, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag);

  /** Set string data and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, const char *stringValue, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag);

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
  bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json);

  /** Set (put) the child (s) nodes and virtual child ".priority" at the defined node.
  */

  bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

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
  bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag);

  /** Set (put) the child (s) nodes and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag);

  /** Set (put) the array to the defined node. 
   * The old content in defined node will be replaced.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which array will be replaced or set.
   * @param arr The pointer to FirebaseJsonArray object.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully stores in the database. 
   * 
   * Call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr will return object and pointer to 
   * FirebaseJsonArray object which contains the array.
  */
  bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);

  /** Set (put) array and virtual child ".priority" at the defined node.
  */
  bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);

  /** Set (put) the array to the defined node if defined node's ETag matched the ETag value. 
   * The old content in defined node will be replaced.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which array will be replaced or set.
   * @param arr The pointer to FirebaseJsonArray object.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 
   * 
   * Call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr will return object and 
   * pointer to FirebaseJsonArray object that contains the array; 
   * 
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
   * 
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 
   * Also call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr to get the array.
  */
  bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag);

  /** Set (put) array and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag);

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
  bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);

  bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);

  /** Set (put) the blob data and virtual child ".priority" at the defined node.
  */
  bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);

  bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);

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
  bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag);

  /** Set (put) the binary data and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag);

  /** Set (put) the binary data from file to the defined node. 
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param path The path to the node in which binary data will be set.
   * @param fileName  The file path includes its name.
   * @return Boolean value, indicates the success of the operation. 
   * 
   * @note No payload returned from the server.
  */
  bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);

  bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);

  /** Set (put) the binary data from file and virtual child ".priority" at the defined node.
  */
  bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);

  bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);

  /** Set (put) the binary data from file to the defined node if defined node's ETag matched the ETag value.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param path The path to the node in which binary data from the file will be set.
   * @param fileName  The file path includes its name.
   * @param ETag Known unique identifier string (ETag) of defined node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note No payload returned from the server. 
   * 
   * If ETag at the defined node does not match the provided ETag parameter,
   * the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).
  */
  bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag);

  bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag);

  /** Set (put) the binary data from the file and the virtual child ".priority" if defined ETag matches at the defined node. 
  */
  bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag);

  bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag);

  /** Set (put) the Firebase server's timestamp to the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node in which timestamp will be set.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].intData will return the integer value of timestamp in seconds 
   * or [FirebaseData object].doubleData to get millisecond timestamp. 
   * 
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
   * use printf("%.0lf\n", firebaseData.doubleData());.
  */
  bool setTimestamp(FirebaseData *fbdo, const char *path);

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
  bool updateNode(FirebaseData *fbdo, const char *path, FirebaseJson *json);

  /** Update (patch) the child (s) nodess and virtual child ".priority" to the defined node.
  */
  bool updateNode(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

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
  bool updateNodeSilent(FirebaseData *fbdo, const char *path, FirebaseJson *json);

  /** Update (patch) the child (s) nodes and virtual child ".priority" to the defined node.
  */
  bool updateNodeSilent(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

  /** Read any type of value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 
   * 
   * Call [FirebaseData object].intData, [FirebaseData object].floatData, [FirebaseData object].doubleData, 
   * [FirebaseData object].boolData, [FirebaseData object].stringData, [FirebaseData object].jsonObject,
   * [FirebaseData object].jsonObjectPtr (pointer), [FirebaseData object].jsonArray,
   * [FirebaseData object].jsonArrayPtr (pointer) and [FirebaseData object].blobData corresponded to 
   * its type that get from [FirebaseData object].dataType.
  */
  bool get(FirebaseData *fbdo, const char *path);

  /** Read (get) the integer value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.

    @return Boolean value, indicates the success of the operation.

    Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 
    
    Call [FirebaseData object].intData will return the integer value of
    payload returned from server.

    If the type of payload returned from server is not integer, float and double, 
    the function [FirebaseData object].intData will return zero (0).

  */
  bool getInt(FirebaseData *fbdo, const char *path);

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
  bool getInt(FirebaseData *fbdo, const char *path, int *target);

  /** Read (get) the float value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * 
   * Call [FirebaseData object].floatData to get float value. 
   * 
   * If the payload returned from server is not integer, float and double, 
   * the function [FirebaseData object].floatData will return zero (0).
  */
  bool getFloat(FirebaseData *fbdo, const char *path);

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
  bool getFloat(FirebaseData *fbdo, const char *path, float *target);

  /** Read (get) the double value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 
   * 
   * Call [FirebaseData object].doubleData to get double value.
   * 
   * If the payload returned from server is not integer, float and double, 
   * the function [FirebaseData object].doubleData will return zero (0).
   * 
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
   * use printf("%.9lf\n", firebaseData.doubleData()); for print value up to 9 decimal places.
  */
  bool getDouble(FirebaseData *fbdo, const char *path);

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
  bool getDouble(FirebaseData *fbdo, const char *path, double *target);

  /** Read (get) the Boolean value at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 
   * 
   * Call [FirebaseData object].boolData to get boolean value.
   * 
   * If the type of payload returned from the server is not Boolean, 
   * the function [FirebaseData object].boolData will return false.
  */
  bool getBool(FirebaseData *fbdo, const char *path);

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
  bool getBool(FirebaseData *fbdo, const char *path, bool *target);

  /** Read (get) the string at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully
   * stores in the database.
   * 
   * Call [FirebaseData object].stringData to get string value.
   * 
   * If the type of payload returned from the server is not a string,
   * the function [FirebaseData object].stringData will return empty string.
  */
  bool getString(FirebaseData *fbdo, const char *path);

  /** Read (get) the string at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @param target The pointer to String object variable to store the value.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note If the type of payload returned from the server is not a string,
   * the target String object's value will be empty.
  */
  bool getString(FirebaseData *fbdo, const char *path, String *target);

  /** Read (get) the child (s) nodes at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully stores in the database.
   * 
   * Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr 
   * to get the JSON data at the defined node.
   * 
   * If the type of payload returned from server is not json,
   * the function [FirebaseData object].jsonObject will contain empty object.
  */
  bool getJSON(FirebaseData *fbdo, const char *path);

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
  bool getJSON(FirebaseData *fbdo, const char *path, FirebaseJson *target);

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
   * Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * 
   * Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr 
   * to get the JSON data at the defined node.
   * 
   * If the type of payload returned from server is not JSON,
   * the function [FirebaseData object].jsonObject will contain empty object.
  */
  bool getJSON(FirebaseData *fbdo, const char *path, QueryFilter *query);

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
  bool getJSON(FirebaseData *fbdo, const char *path, QueryFilter *query, FirebaseJson *target);

  /** Read (get) the array at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully
   * stores in the database.
   * 
   * Call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr will return object and 
   * pointer to FirebaseJsonArray object that contains the array; 
   * 
   * If the type of payload returned from the server is not an array,
   * the array element in [FirebaseData object].jsonArray will be empty.
  */
  bool getArray(FirebaseData *fbdo, const char *path);

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
  bool getArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *target);

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
   * Call [FirebaseData object].dataType to determine what type of data that successfully 
   * stores in the database.
   * 
   * Call [FirebaseData object].jsonArray will return the pointer to FirebaseJsonArray object contains array of 
   * payload returned from server.
   * 
   * If the type of payload returned from the server is not an array,
   * the function [FirebaseData object].jsonArray will contain empty array.
  */
  bool getArray(FirebaseData *fbdo, const char *path, QueryFilter *query);

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
  bool getArray(FirebaseData *fbdo, const char *path, QueryFilter *query, FirebaseJsonArray *target);

  /** Read (get) the blob (binary data) at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * 
   * Call [FirebaseData object].blobData to get the uint8_t vector.
   * 
   * If the type of payload returned from the server is not a blob,
   * the function [FirebaseData object].blobData will return empty array.
  */
  bool getBlob(FirebaseData *fbdo, const char *path);

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
  bool getBlob(FirebaseData *fbdo, const char *path, std::vector<uint8_t> *target);

  /** Download file data at the defined node and save to storage memory.
   *  
   * The downloaded data will be decoded to binary and save to SD card/Flash memory, 
   * then please make sure that data at the defined node is the file type.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param nodePath The path to the node that file data will be downloaded.
   * @param fileName  The file path includes its name.
   * @return Boolean value, indicates the success of the operation.
  */
  bool getFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName);

  /** Delete all child nodes at the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node to be deleted.
   * @return Boolean value, indicates the success of the operation.
  */
  bool deleteNode(FirebaseData *fbdo, const char *path);

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
  bool deleteNode(FirebaseData *fbdo, const char *path, const char *ETag);

  /** Subscribe to the value changes on the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param path The path to the node to subscribe.
   * @return Boolean value, indicates the success of the operation.
  */
  bool beginStream(FirebaseData *fbdo, const char *path);

  /** Subscribe to the value changes on the children of the defined node.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param parentPath The path to the parent node to subscribe.
   * @param childPath The string array of the path to child nodes.
   * @param size The size of string array of the path to the child nodes.
   * @return Boolean value, indicates the success of the operation.
  */
  bool beginMultiPathStream(FirebaseData *fbdo, const char *parentPath, const String *childPath, size_t size);

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
   * @param dataAvailableCallback The Callback function that accepts FirebaseStream parameter.
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
   * Call [FirebaseStream object].dataType to determine what type of data successfully stores in the database. 
   * 
   * Call [FirebaseStream object].xxxData will return the appropriate data type of 
   * the payload returned from the server.
  */
#if defined(ESP32)
  void setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize = 8192);
#elif defined(ESP8266)
  void setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback);
#endif
  /** Set the multiple paths stream callback functions. 
   * setMultiPathStreamCallback should be called before Firebase.beginMultiPathStream.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param multiPathDataCallback The Callback function that accepts MultiPathStream parameter.
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
   * Call [MultiPathStream object].get to get the child node value, type and its data path. 
   * 
   * The properties [MultiPathStream object].value, [MultiPathStream object].dataPath, and [MultiPathStream object].type will return the value, path of data, and type of data respectively.
   * 
   * These properties will store the result from calling the function [MultiPathStream object].get.
  */
#if defined(ESP32)
  void setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback = NULL, size_t streamTaskStackSize = 8192);
#elif defined(ESP8266)
  void setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback = NULL);
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

  /** Backup (download) the database at the defined node to the storage memory.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param nodePath The path to the node to be backuped.
   * @param fileName File name to save.
   * @return Boolean value, indicates the success of the operation.
   * 
   * @note Only 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension) can be saved to SD card/Flash memory.
  */
  bool backup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName);

  /** Restore the database at a defined path using backup file saved on SD card/Flash memory.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @param nodePath The path to the node to be restored the data.
   * @param fileName File name to read.
   * @return Boolean value, indicates the success of the operation.
  */
  bool restore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName);

  /** Set maximum Firebase read/store retry operation (0 - 255) 
   * in case of network problems and buffer overflow.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param num The maximum retry.
  */
  void setMaxRetry(FirebaseData *fbdo, uint8_t num);

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
  bool saveErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType);

  /** Delete file in storage memory.
   * 
   * @param filename File name to delete.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
  */
  bool deleteStorageFile(const char *filename, fb_esp_mem_storage_type storageType);

  /** Restore the Firebase Error Queues from the queue file (flash memory).
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param filename Filename to be read and restore queues.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
  */
  bool restoreErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType);

  /** Determine the number of Firebase Error Queues stored in a defined file (flash memory).
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param filename Filename to be read and count for queues.
   * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
   * @return Number (0-255) of queues store in defined queue file.
  */
  uint8_t errorQueueCount(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType);

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
   * queueInfo.currentQueueID(), get current Error Queue ID that being process.
   * 
   * queueInfo.isQueueFull(), determine whether Error Queue Collection is full or not.
   * 
   * queueInfo.dataType(), get a string of the Firebase call data type that being process of current Error Queue.
   * 
   * queueInfo.method(), get a string of the Firebase call method that being process of current Error Queue.
   * 
   * queueInfo.path(), get a string of the Firebase call path that being process of current Error Queue.
  */
#if defined(ESP32)
  void beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback = NULL, size_t queueTaskStackSize = 8192);
#elif defined(ESP8266)
  void beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback = NULL);
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

  template <typename T>
  bool push(FirebaseData *fbdo, const char *path, T value);

  template <typename T>
  bool push(FirebaseData *fbdo, const char *path, T value, size_t size);

  template <typename T>
  bool push(FirebaseData *fbdo, const char *path, T value, float priority);

  template <typename T>
  bool push(FirebaseData *fbdo, const char *path, T value, size_t size, float priority);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value, size_t size);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value, float priority);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value, size_t size, float priority);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value, const char *ETag);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value, size_t size, const char *ETag);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value, float priority, const char *ETag);

  template <typename T>
  bool set(FirebaseData *fbdo, const char *path, T value, size_t size, float priority, const char *ETag);

private:
  UtilsClass *ut = nullptr;
  void begin(UtilsClass *u);
  void rescon(FirebaseData *fbdo, const char *host, fb_esp_rtdb_request_info_t *req);
  void clearDataStatus(FirebaseData *fbdo);
  bool handleRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  int sendRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  void preparePayload(struct fb_esp_rtdb_request_info_t *req, std::string &buf);
  void prepareHeader(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req, int payloadLength, std::string &header, bool sv);
  bool waitResponse(FirebaseData *fbdo);
  bool handleResponse(FirebaseData *fbdo);
  void handlePayload(FirebaseData *fbdo, struct server_response_data_t &response, const char *payload);
  bool processRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  bool processRequestFile(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req);
  bool pushInt(FirebaseData *fbdo, const char *path, int intValue, bool queue, const char *priority);
  bool pushFloat(FirebaseData *fbdo, const char *path, float floatValue, bool queue, const char *priority);
  bool pushDouble(FirebaseData *fbdo, const char *path, double doubleValue, bool queue, const char *priority);
  bool pushBool(FirebaseData *fbdo, const char *path, bool boolValue, bool queue, const char *priority);
  bool pushString(FirebaseData *fbdo, const char *path, const String &stringValue, const char *priority);
  bool pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *priority);
  bool pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *priority);
  bool pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, bool queue, const char *priority);
  bool pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *priority);
  bool setInt(FirebaseData *rtdb, const char *path, int intValue, bool queue, const char *priority, const char *etag);
  bool setFloat(FirebaseData *rtdb, const char *path, float floatValue, bool queue, const char *priority, const char *etag);
  bool setDouble(FirebaseData *rtdb, const char *path, double doubleValue, bool queue, const char *priority, const char *etag);
  bool setBool(FirebaseData *rtdb, const char *path, bool boolValue, bool queue, const char *priority, const char *etag);
  bool setString(FirebaseData *fbdo, const char *path, const char *stringValue, const char *priority, const char *etag);
  bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *priority, const char *etag);
  bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *priority, const char *ETag);
  bool setBlob(FirebaseData *rtdb, const char *path, uint8_t *blob, size_t size, bool queue, const char *priority, const char *etag);
  bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *priority, const char *etag);
  bool handleStreamRequest(FirebaseData *fbdo, const std::string &path);
  bool connectionError(FirebaseData *fbdo);
  bool handleStreamRead(FirebaseData *fbdo);
  void sendCB(FirebaseData *fbdo);
  void splitStreamPayload(const char *payloads, std::vector<std::string> &payload);
  void parseStreamPayload(FirebaseData *fbdo, const char *payload);
#if defined(ESP32)
  void runStreamTask(FirebaseData *fbdo, const char *taskName);
#elif defined(ESP8266)
  void runStreamTask();
  void runErrorQueueTask();
#endif
  uint8_t openErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType, uint8_t mode);
};

#endif