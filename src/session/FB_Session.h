/**
 * Google's Firebase Data class, FB_Session.h version 1.0.11
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created June 10, 2021
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

#ifndef FIREBASE_SESSION_H
#define FIREBASE_SESSION_H
#include <Arduino.h>
#include "Utils.h"
#include "stream/FB_Stream.h"
#include "stream/FB_MP_Stream.h"
#include "rtdb/QueueInfo.h"
#include "rtdb/QueueManager.h"

#include "signer/Signer.h"

#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)

enum fb_esp_fcm_msg_type
{
  msg_single,
  msg_multicast,
  msg_topic
};

class FCMObject
{

#if defined(ESP32)
  friend class FirebaseESP32;
#elif defined(ESP8266)
  friend class FirebaseESP8266;
#endif
  friend class FirebaseData;

public:
  FCMObject();
  ~FCMObject();

  /** Store Firebase Cloud Messaging's authentication credentials.
   * 
   * @param serverKey Server key found on Console: Project settings > Cloud Messaging
   */
  void begin(const String &serverKey);

  /** Add recipient's device registration token or instant ID token.
   * 
   * @param deviceToken Recipient's device registration token to add that message will be sent to.
   */
  void addDeviceToken(const String &deviceToken);

  /** Remove the recipient's device registration token or instant ID token.
   * 
   * @param index Index (start from zero) of the recipient's device registration token that added to FCM Data Object of Firebase Data object.
   */
  void removeDeviceToken(uint16_t index);

  /** Clear all recipient's device registration tokens.
  */
  void clearDeviceToken();

  /** Set the notify message type information.
   * 
   * @param title The title text of notification message.
   * @param body The body text of notification message.
   */
  void setNotifyMessage(const String &title, const String &body);

  /** Set the notify message type information.
   * 
   * @param title The title text of notification message.
   * @param body The body text of notification message.
   * @param icon The name and/or included URI/URL of the icon to show on notifying message.
   */
  void setNotifyMessage(const String &title, const String &body, const String &icon);

  /** Set the notify message type information.
   * 
   * @param title The title text of notification message.
   * @param body The body text of notification message.
   * @param icon The name and/or included URI/URL of the icon to show on notifying message.
   * @param click_action The URL or intent to accept click event on the notification message.
   */
  void setNotifyMessage(const String &title, const String &body, const String &icon, const String &click_action);

  /** add the custom key/value in the notify message type information.
   * 
   * @param key The key field in notification message.
   * @param value The value field in the notification message.
  */
  void addCustomNotifyMessage(const String &key, const String &value);

  /** Clear all notify message information.
  */
  void clearNotifyMessage();

  /** Set the custom data message type information.
   * 
   * @param jsonString The JSON structured data string.
  */
  void setDataMessage(const String &jsonString);

  /** Set the custom data message type information.
   * 
   * @param json The FirebaseJson object.
  */
  void setDataMessage(FirebaseJson &json);

  /** Clear custom data message type information.
  */
  void clearDataMessage();

  /** Set the priority of the message (notification and custom data).
   * 
   * @param priority The priority string i.e. normal and high.
  */
  void setPriority(const String &priority);

  /** Set the collapse key of the message (notification and custom data).
   * 
   * @param key String of collapse key.
  */
  void setCollapseKey(const String &key);

  /** Set the Time To Live of the message (notification and custom data).
   * 
   * @param seconds Number of seconds from 0 to 2,419,200 (4 weeks).
  */
  void setTimeToLive(uint32_t seconds);

  /** Set the topic of the message will be sent to.
   * 
   * @param topic Topic string.
  */
  void setTopic(const String &topic);

  /** Get the send result.
   * 
   * @return String of payload returned from the server.
  */
  String getSendResult();

private:
  bool init();

  void begin(UtilsClass *u);

  bool waitResponse(FirebaseData &fbdo);

  bool handleResponse(FirebaseData *fbdo);

  void rescon(FirebaseData &fbdo, const char *host);

  void fcm_begin(FirebaseData &fbdo);

  bool fcm_send(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType);

  void fcm_prepareHeader(std::string &header, size_t payloadSize);

  void fcm_preparePayload(std::string &msg, fb_esp_fcm_msg_type messageType);

  void clear();

  std::string _topic = "";
  std::string _server_key = "";
  std::string _sendResult = "";
  FirebaseJson _fcmPayload;
  int _ttl = -1;
  uint16_t _index = 0;
  uint16_t _port = FIREBASE_PORT;
  std::vector<std::string> _deviceToken;
  UtilsClass *_ut = nullptr;
  FirebaseAuth _auth_;
  FirebaseConfig _cfg_;
};

#endif

class FirebaseData
{
  friend class FB_RTDB;
  friend class UtilsClass;
#if defined(FIREBASE_ESP_CLIENT)
  friend class FB_CM;
  friend class GG_CloudStorage;
  friend class FB_Storage;
  friend class FB_Firestore;
  friend class FB_Functions;
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#if defined(ESP32)
  friend class FirebaseESP32;
#elif defined(ESP8266)
  friend class FirebaseESP8266;
#endif
  friend class FCMObject;
#endif

public:
  typedef void (*StreamEventCallback)(FIREBASE_STREAM_CLASS);
  typedef void (*MultiPathStreamEventCallback)(FIREBASE_MP_STREAM_CLASS);
  typedef void (*StreamTimeoutCallback)(bool);
  typedef void (*QueueInfoCallback)(QueueInfo);

  FirebaseData();
  ~FirebaseData();

#if defined(ESP8266)
  /** Set the receive and transmit buffer memory size for secured mode BearSSL WiFi client.
   * 
   * @param rx The number of bytes for receive buffer memory for secured mode BearSSL (512 is minimum, 16384 is maximum).
   * @param tx The number of bytes for transmit buffer memory for secured mode BearSSL (512 is minimum, 16384 is maximum).
   * 
   * @note Set this option to false to support get large Blob and File operations.
  */
  void setBSSLBufferSize(uint16_t rx, uint16_t tx);
#endif

  /** Set the HTTP response size limit.
   * 
   * @param len The server response buffer size limit. 
  */
  void setResponseSize(uint16_t len);

  /** Pause/Unpause WiFiClient from all Firebase operations.
   * 
   * @param pause The boolean to set/unset pause operation.
   * @return Boolean type status indicates the success of operation.
  */
  bool pauseFirebase(bool pause);

  /** Check the pause status of Firebase Data object.
   * 
   * @return Boolean type value of pause status.
  */
  bool isPause();

  /** Get a WiFi client instance.
   * 
   * @return WiFi client instance.
  */
  WiFiClientSecure *getWiFiClient();

  /** Close the keep-alive connection of the internal WiFi client.
   * 
   * @note This will release the memory used by internal WiFi client.
  */
  void stopWiFiClient();

  /** Get the data type of payload returned from the server (RTDB only).
   * 
   * @return The one of these data type e.g. integer, float, double, boolean, string, JSON and blob.
  */
  String dataType();

  /** Get the event type of stream.
   * 
   * @return The one of these event type e.g. put, patch, cancel, and auth_revoked.
   * 
   * @note The event type "put" indicated that data at an event path relative to the stream path was completely changed. The event path can be Getd by dataPath().
   * 
   * The event type "patch" indicated that data at the event path relative to stream path was updated. The event path can be Getd by dataPath().
   * 
   * The event type "cancel" indicated something wrong and cancel by the server.
   * 
   * The event type "auth_revoked" indicated the provided Firebase Authentication Data (Database secret) is no longer valid.
  */
  String eventType();

  /** Get the unique identifier (ETag) of current data.
   * 
   * @return String of unique identifier.
  */
  String ETag();

  /** Get the current stream path.
   * 
   * @return The database streaming path.
  */
  String streamPath();

  /** Get the current data path.
   * 
   * @return The database path which belongs to the server's returned payload.
   * 
   * @note The database path returned from this function in case of stream, also changed upon the child or parent's stream 
   * value changes.
  */
  String dataPath();

#if defined(FIREBASE_ESP_CLIENT)

  /** Get the metadata of a file in the Firebase storage data bucket.
   * 
   * @return The FileMetaInfo data of file.
   * 
   * @note The FileMetaInfo properties are
   * name - The file name
   * bucket - The storage bucket id
   * generation - The timestamp (millisecond) of file
   * contentType - The content type or mime type of file
   * size - The size of file in byte
   * etag - The ETag of file
   * crc32c - The CRC32 of file
   * downloadTokens - The download token
  */
  FileMetaInfo metaData();

  /** Get the files info in the Firebase storage data bucket.
   * 
   * @return The pointer to FileItem array (FileList) of files in the data bucket.
   * 
   * @note The FileItem properties are
   * name - The file name
   * bucket - The storage bucket id
  */
  FileList *fileList();

  /** Get the download URL from the currently uploaded file in the Firebase storage data bucket.
   * 
   * @return The URL to download file.
  */
  String downloadURL();

#endif

  /** Get the error reason String from the process.
   * 
   * @return The error description string (String object).
  */
  String errorReason();

  /** Return the integer data of server returned payload.
   * 
   * @return integer value.
  */
  int intData();

  /** Return the float data of server returned payload.
   * 
   * @return Float value.
  */
  float floatData();

  /** Return the double data of server returned payload.
   * 
   * @return double value.
  */
  double doubleData();

  /** Return the Boolean data of server returned payload.
   * 
   * @return Boolean value.
  */
  bool boolData();

  /** Return the String data of server returned payload.
   * 
   * @return String (String object).
  */
  String stringData();

  /** Return the JSON String data of server returned payload.
   * 
   * @return String (String object).
  */
  String jsonString();

  /** Return the Firebase JSON object of server returned payload.
   * 
   * @return FirebaseJson object.
  */
  FirebaseJson &jsonObject();

  /** Return the Firebase JSON object pointer of server returned payload.
   * 
   * @return FirebaseJson object pointer.
  */
  FirebaseJson *jsonObjectPtr();

  /** Return the Firebase JSON Array object of server returned payload.
   * 
   * @return FirebaseJsonArray object.
  */
  FirebaseJsonArray &jsonArray();

  /** Return the Firebase JSON Array object pointer of server returned payload.
   * 
   * @return FirebaseJsonArray object pointer.
  */
  FirebaseJsonArray *jsonArrayPtr();

  /** Return the Firebase JSON Data object that keeps the get(parse) result.
   * 
   * @return FirebaseJsonData object.
  */
  FirebaseJsonData &jsonData();

  /** Return the Firebase JSON Data object pointer that keeps the get(parse) result.
   * 
   * @return FirebaseJsonData object pointer.
  */
  FirebaseJsonData *jsonDataPtr();

  /** Return the blob data (uint8_t) array of server returned payload.
   * 
   * @return Dynamic array of 8-bit unsigned integer i.e. std::vector<uint8_t>.
  */
  std::vector<uint8_t> blobData();

  /** Return the file stream of server returned payload.
   * 
   * @return the file stream.
  */
  fs::File fileStream();

  /** Return the new appended node's name or key of server returned payload when calling pushXXX function.
   * 
   * @return String (String object).
  */
  String pushName();

  /** Get the stream connection status.
   * 
   * @return Boolean type status indicates whether the Firebase Data object is working with a stream or not.
  */
  bool isStream();

  /** Get the server connection status.
   * 
   * @return Boolean type status indicates whether the Firebase Data object is connected to the server or not.
  */
  bool httpConnected();

  /** Get the timeout event of the server's stream (30 sec is the default). 
   * Nothing to do when stream connection timeout, the stream connection will be automatically resumed.
   * 
   * @return Boolean type status indicates whether the stream was a timeout or not.
  */
  bool streamTimeout();

  /** Get the availability of data or payload returned from the server.
   * 
   * @return Boolean type status indicates whether the server return the new payload or not.
  */
  bool dataAvailable();

  /** Get the availability of stream event-data payload returned from the server.
   * 
   * @return Boolean type status indicates whether the server returns the stream event-data 
   * payload or not.
  */
  bool streamAvailable();

  /** Get the matching between data type that intends to get from/store to database and the server's return payload data type.
   * 
   * @return Boolean type status indicates whether the type of data being get from/store to database 
   * and the server's returned payload is matched or not.
  */
  bool mismatchDataType();

  /** Get the data type of payload returned from the server (RTDB only).
   * 
   * @return The enumeration value of fb_esp_rtdb_data_type.
   * fb_esp_rtdb_data_type_null or 1,
   * fb_esp_rtdb_data_type_integer or 2,
   * fb_esp_rtdb_data_type_float or 3,
   * fb_esp_rtdb_data_type_double or 4,
   * fb_esp_rtdb_data_type_boolean or 5,
   * fb_esp_rtdb_data_type_string or 6,
   * fb_esp_rtdb_data_type_json or 7,
   * fb_esp_rtdb_data_type_array or 8,
   * fb_esp_rtdb_data_type_blob or 9,
   * fb_esp_rtdb_data_type_file or 10
  */
  uint8_t dataTypeEnum();

  /** Get the HTTP status code return from the server.
   * 
   * @return integer number of HTTP status.
  */
  int httpCode();

  /** Check the overflow of the returned payload data buffer.
   * 
   * @return The overflow status. 
   * 
   * @note Total default HTTP response buffer size is 400 bytes which can be set through FirebaseData.setResponseSize.
  */
  bool bufferOverflow();

  /** Get the name (full path) of the backup file in SD card/Flash memory.
   * 
   * @return String (String object) of the file name that stores on SD card/Flash memory after backup operation.
  */
  String getBackupFilename();

  /** Get the size of the backup file.
   * 
   * @return Size of backup file in byte after backup operation.
  */
  size_t getBackupFileSize();

  /** Clear or empty data in Firebase Data object.
  */
  void clear();

  /** Get the error description for file transferring (push file, set file, backup and restore).
   * 
   * @return Error description string (String object).
  */
  String fileTransferError();

  /** Return the server's payload data.
   * 
   * @return Payload string (String object).
   * 
   * @note The returned String will be empty when the response data is File, BLOB, JSON and JSON Array objects.
   * 
   * For File data type, call fileStream to get the file stream.
   * 
   * For BLOB data type, call blobData to get the dynamic array of unsigned 8-bit data.
   * 
   * For JSON object data type, call jsonObject and jsonObjectPtr to get the object and its pointer.
   * 
   * For JSON Array data type, call jsonArray and jsonArrayPtr to get the object and its pointer.
  */
  String payload();

#if defined(ESP32)
  FB_HTTPClient32 httpClient;
#elif defined(ESP8266)
  FB_HTTPClient httpClient;
#endif

  QueryFilter queryFilter;
#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
  FCMObject fcm;
#endif

private:
  StreamEventCallback _dataAvailableCallback = NULL;
  MultiPathStreamEventCallback _multiPathDataCallback = NULL;
  StreamTimeoutCallback _timeoutCallback = NULL;
  QueueInfoCallback _queueInfoCallback = NULL;
#if defined(FIREBASE_ESP_CLIENT)
  FunctionsOperationCallback _functionsOperationCallback = NULL;
#endif

  UtilsClass *ut = nullptr;
  QueueManager _qMan;
  struct fb_esp_session_info_t _ss;

  void closeSession();
  bool handleStreamRead();
  void checkOvf(size_t len, struct server_response_data_t &resp);
  bool reconnect(unsigned long dataTime = 0);
  std::string getDataType(uint8_t type);
  std::string getMethod(uint8_t method);
  bool tokenReady();
  void setSecure();
  bool validRequest(const std::string &path);
  void addQueue(struct fb_esp_rtdb_queue_info_t *qinfo);

  void clearQueueItem(QueueItem *item);

  void setQuery(QueryFilter *query);

  void clearNodeList();

  void addNodeList(const String *childPath, size_t size);

  bool init();
};

#endif