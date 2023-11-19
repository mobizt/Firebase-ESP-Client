
/**
 * Google's Firebase Data class, FB_Session.h version 1.4.1
 *
 * Created September 12, 2023
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

#ifndef FIREBASE_SESSION_H
#define FIREBASE_SESSION_H

#include <Arduino.h>
#include "./mbfs/MB_MCU.h"
#include "./FirebaseFS.h"
#include "./FB_Utils.h"
#include "./core/FirebaseCore.h"

#include "./rtdb/stream/FB_Stream.h"
#include "./rtdb/stream/FB_MP_Stream.h"
#include "./rtdb/QueueInfo.h"
#include "./rtdb/QueueManager.h"

#if defined(ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_ARCH_SAMD)
#if __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#endif
#endif
/**
 * Simple Queue implemented in this library is for error retry only.
 * Other QueueTask management e.g., FreeRTOS Queue is not necessary.
 */

using namespace mb_string;

#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)
#include "./message/LFCM.h"
#endif
#endif

class FirebaseData
{

  friend class FIREBASE_CLASS;

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  friend class FB_RTDB;
#endif

#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)
  friend class FB_CM;
#endif

#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
  friend class FB_Storage;
#endif

#if defined(ENABLE_FIRESTORE) || defined(FIREBASE_ENABLE_FIRESTORE)
  friend class FB_Firestore;
#endif

#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)
  friend class FB_Functions;
#endif

#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
  friend class GG_CloudStorage;
#endif

public:
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  typedef void (*StreamEventCallback)(FIREBASE_STREAM_CLASS);
  typedef void (*MultiPathStreamEventCallback)(FIREBASE_MP_STREAM_CLASS);
  typedef void (*StreamTimeoutCallback)(bool);
  typedef void (*QueueInfoCallback)(QueueInfo);
#endif

#if !defined(FIREBASE_ESP32_CLIENT) && !defined(FIREBASE_ESP8266_CLIENT) && (defined(ENABLE_FIRESTORE) || defined(FIREBASE_ENABLE_FIRESTORE))
  typedef void (*FirestoreBatchOperationsCallback)(const char *);
#endif

  FirebaseData();
  ~FirebaseData();

  /** Assign external Arduino Client.
   *
   * @param client The pointer to Arduino Client derived class of SSL Client.
   */
  FirebaseData(Client *client);

  /** Assign external Arduino generic client.
   *
   * @param client The pointer to Arduino Client.
   * @param networkConnectionCB The function that handles the network connection.
   * @param networkStatusCB The function that handle the network connection status acknowledgement.
   */
  void setGenericClient(Client *client, FB_NetworkConnectionRequestCallback networkConnectionCB,
                        FB_NetworkStatusRequestCallback networkStatusCB);

  /** Assign external Arduino generic client (deprecated, use setGenericClient instead).
   *
   * @param client The pointer to Arduino Client derived class of SSL Client.
   */
  void setExternalClient(Client *client) { setGenericClient(client, nullptr, nullptr); };

  /** Assign the callback functions required for external Client usage (deprecated, use setGenericClient instead).
   *
   * @param networkConnectionCB The function that handles the network connection.
   * @param networkStatusCB The function that handle the network connection status acknowledgement.
   */

  void setExternalClientCallbacks(FB_NetworkConnectionRequestCallback networkConnectionCB,
                                  FB_NetworkStatusRequestCallback networkStatusCB) { setGenericClient(nullptr, networkConnectionCB, networkStatusCB); };

  /** Assign the callback functions required for external Client usage (deprecated, use setGenericClient instead).
   *
   * @param tcpConnectionCB The function that handles the server connection.
   * @param networkConnectionCB The function that handles the network connection.
   * @param networkStatusCB The function that handle the network connection status acknowledgement.
   */
  void setExternalClientCallbacks(FB_TCPConnectionRequestCallback tcpConnectionCB,
                                  FB_NetworkConnectionRequestCallback networkConnectionCB,
                                  FB_NetworkStatusRequestCallback networkStatusCB) { setGenericClient(nullptr, networkConnectionCB, networkStatusCB); };

  /** Assign external TinyGsm Client.
   *
   * @param client The pointer to TinyGsmClient.
   * @param modem The pointer to TinyGsm modem object. Modem should be initialized and/or set mode before transfering data.
   * @param pin The SIM pin.
   * @param apn The GPRS APN (Access Point Name).
   * @param user The GPRS user.
   * @param password The GPRS password.
   */
  void setGSMClient(Client *client, void *modem, const char *pin, const char *apn, const char *user, const char *password);

  /** Assign external Ethernet Client.
   *
   * @param client The pointer to Ethernet client object.
   * @param macAddress The Ethernet MAC address.
   * @param csPin The Ethernet module SPI chip select pin.
   * @param resetPin The Ethernet module reset pin.
   * @param staticIP (Optional) The pointer to Firebase_StaticIP object which included these IPAddress properties
   * ipAddress, netMask, defaultGateway and dnsServer.
   */
  void setEthernetClient(Client *client, uint8_t macAddress[6], int csPin, int resetPin, Firebase_StaticIP *staticIP = nullptr);

  /** Set the network status acknowledgement.
   *
   * @param status The network status.
   */
  void setNetworkStatus(bool status);

  /** Set the receive and transmit buffer memory size for secured mode BearSSL WiFi client.
   *
   * @param rx The number of bytes for receive buffer memory for secured mode BearSSL (512 is minimum, 16384 is maximum).
   * @param tx The number of bytes for transmit buffer memory for secured mode BearSSL (512 is minimum, 16384 is maximum).
   *
   * @note Set this option to false to support get large Blob and File operations.
   */
  void setBSSLBufferSize(uint16_t rx, uint16_t tx);

  /** Set the HTTP response size limit.
   *
   * @param len The server response buffer size limit.
   */
  void setResponseSize(uint16_t len);

  /** Set the Root certificate for a FirebaseData object.
   *
   * @param ca PEM format certificate string.
   */
  void setCert(const char *ca);

  /** Pause/Unpause WiFiClient from all Firebase operations.
   *
   * @param pause The boolean to set/unset pause operation.
   * @return Boolean type status indicates the success of operation.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool pauseFirebase(bool pause);
#endif

  /** Check the pause status of FirebaseData object.
   *
   * @return Boolean type value of pause status.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool isPause();
#endif

  /** Get a WiFi client instance.
   *
   * @return WiFi client instance.
   */
  ESP_SSLClient *getWiFiClient();

  /** Close the keep-alive connection of the internal SSL client.
   *
   * @note This will release the memory used by internal SSL client.
   */
  void stopWiFiClient();

  /** Close the internal flash temporary file.
   *
   */
  void closeFile();

  /** Get the data type of payload returned from the server (RTDB only).
   *
   * @return The one of these data type e.g. string, boolean, int, float, double, json, array, blob, file and null.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String dataType();
#endif

  /** Get the data type of payload returned from the server (RTDB only).
   *
   * @return The enumeration value of firebase_rtdb_data_type.
   * firebase_rtdb_data_type_null or 1,
   * firebase_rtdb_data_type_integer or 2,
   * firebase_rtdb_data_type_float or 3,
   * firebase_rtdb_data_type_double or 4,
   * firebase_rtdb_data_type_boolean or 5,
   * firebase_rtdb_data_type_string or 6,
   * firebase_rtdb_data_type_json or 7,
   * firebase_rtdb_data_type_array or 8,
   * firebase_rtdb_data_type_blob or 9,
   * firebase_rtdb_data_type_file or 10
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  uint8_t dataTypeEnum();
#endif

  /** Get the event type of stream. (RTDB only)
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
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String eventType();
#endif

  /** Get the unique identifier (ETag) of RTDB data. (RTDB only)
   *
   * @return String of unique identifier.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String ETag();
#endif

  /** Get the current stream path. (RTDB only)
   *
   * @return The database streaming path.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String streamPath();
#endif

  /** Get the current data path. (RTDB only)
   *
   * @return The database path which belongs to the server's returned payload.
   *
   * @note The database path returned from this function in case of stream, also changed upon the child or parent's stream
   * value changes.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String dataPath();
#endif
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
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
  FileMetaInfo metaData();
#endif

  /** Get the files info in the Firebase storage data bucket.
   *
   * @return The pointer to FileItem array (FileList) of files in the data bucket.
   *
   * @note The FileItem properties are
   * name - The file name
   * bucket - The storage bucket id
   */
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
  FileList *fileList();
#endif

  /** Get the download URL from the currently uploaded file in the Firebase storage data bucket.
   *
   * @return The URL to download file.
   */
#if (defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)) || defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
  String downloadURL();
#endif

#endif

  /** Get the error reason String from the process.
   *
   * @return The error description string (String object).
   */
  String errorReason();

  /** Get the error code from the process.
   *
   * @return The error code (int).
   *
   * See src/FB_Error.h
   *
   */
  int errorCode();

  /** Return the integer data of server returned payload (RTDB only).
   *
   * @return integer value.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  int intData();
#endif

  /** Return the float data of server returned payload (RTDB only).
   *
   * @return Float value.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  float floatData();
#endif

  /** Return the double data of server returned payload (RTDB only).
   *
   * @return double value.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  double doubleData();
#endif

  /** Return the Boolean data of server returned payload (RTDB only).
   *
   * @return Boolean value.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool boolData();
#endif

  /** Return the String data of server returned payload (RTDB only).
   *
   * @return String (String object).
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String stringData();
#endif

  /** Return the JSON String data of server returned payload (RTDB only).
   *
   * @return String (String object).
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String jsonString();
#endif

  /** Return the Firebase JSON object of server returned payload (RTDB only).
   *
   * @return FirebaseJson object.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  FirebaseJson &jsonObject();
#endif

  /** Return the Firebase JSON object pointer of server returned payload (RTDB only).
   *
   * @return FirebaseJson object pointer.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  FirebaseJson *jsonObjectPtr();
#endif

  /** Return the Firebase JSON Array object of server returned payload (RTDB only).
   *
   * @return FirebaseJsonArray object.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  FirebaseJsonArray &jsonArray();
#endif

  /** Return the Firebase JSON Array object pointer of server returned payload (RTDB only).
   *
   * @return FirebaseJsonArray object pointer.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  FirebaseJsonArray *jsonArrayPtr();
#endif

  /** Return the internal Firebase JSON Data object.
   *
   * @return FirebaseJsonData object.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  FirebaseJsonData &jsonData();
#endif

  /** Return the pointer to internal Firebase JSON Data object.
   *
   * @return FirebaseJsonData object pointer..
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  FirebaseJsonData *jsonDataPtr();
#endif

  /** Return the pointer to blob data (uint8_t) array of server returned payload (RTDB only).
   *
   * @return Dynamic array of 8-bit unsigned integer i.e. std::vector<uint8_t>.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  MB_VECTOR<uint8_t> *blobData();
#endif

  /** Return the file stream of server returned payload (RTDB only).
   *
   * @return the file stream.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
#if defined(MBFS_FLASH_FS)
  File fileStream();
#endif
#endif

  /**
   * Get the value by specific type from FirebaseJsonData object (RTDB only).
   * This should call after parse or get function.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  template <typename T>
  auto to() -> typename enable_if<is_num_int<T>::value || is_num_float<T>::value || is_bool<T>::value, T>::type
  {
    if (session.rtdb.resp_data_type == firebase_data_type::d_string)
      setRaw(true); // if double quotes string, trim it.

    if (session.rtdb.raw.length() > 0)
    {
      if (session.rtdb.resp_data_type == firebase_data_type::d_boolean)
        mSetBoolValue(strcmp(session.rtdb.raw.c_str(), num2Str(true, -1)) == 0);
      else if (session.rtdb.resp_data_type == firebase_data_type::d_integer ||
               session.rtdb.resp_data_type == firebase_data_type::d_float ||
               session.rtdb.resp_data_type == firebase_data_type::d_double)
      {
        mSetIntValue(session.rtdb.raw.c_str());
        mSetFloatValue(session.rtdb.raw.c_str());
      }
    }

    if (session.rtdb.req_data_type == d_timestamp)
    {
      if (is_num_uint64<T>::value)
        return iVal.uint64;
      if (is_num_int32<T>::value || is_num_uint32<T>::value || is_num_int64<T>::value || is_num_uint64<T>::value)
        return iVal.uint64 / 1000;
      else
        return 0;
    }

    if (is_bool<T>::value)
      return iVal.int32 > 0;
    else if (is_num_int8<T>::value)
      return iVal.int8;
    else if (is_num_uint8<T>::value)
      return iVal.uint8;
    else if (is_num_int16<T>::value)
      return iVal.int16;
    else if (is_num_uint16<T>::value)
      return iVal.uint16;
    else if (is_num_int32<T>::value)
      return iVal.int32;
    else if (is_num_uint32<T>::value)
      return iVal.uint32;
    else if (is_num_int64<T>::value)
      return iVal.int64;
    else if (is_num_uint64<T>::value)
      return iVal.uint64;
    else if (is_same<T, float>::value)
      return fVal.f;
    else if (is_same<T, double>::value)
      return fVal.d;
    else
      return 0;
  }

  template <typename T>
  auto to() -> typename enable_if<is_const_chars<T>::value || is_std_string<T>::value || is_arduino_string<T>::value || is_mb_string<T>::value, T>::type
  {
    if (session.rtdb.resp_data_type == firebase_data_type::d_string)
      setRaw(true);
    return session.rtdb.raw.c_str();
  }

  template <typename T>
  auto to() -> typename enable_if<is_same<T, FirebaseJson *>::value, FirebaseJson *>::type
  {
    if (!session.jsonPtr)
      session.jsonPtr = new FirebaseJson();

    if (session.rtdb.resp_data_type == d_json)
    {
      session.jsonPtr->clear();
      if (session.arrPtr)
        session.arrPtr->clear();
      session.jsonPtr->setJsonData(session.rtdb.raw.c_str());
    }
    return session.jsonPtr;
  }

  template <typename T>
  auto to() -> typename enable_if<is_same<T, FirebaseJsonData *>::value, FirebaseJsonData *>::type
  {
    if (!session.dataPtr)
      session.dataPtr = new FirebaseJsonData();
    return session.dataPtr;
  }

  template <typename T>
  auto to() -> typename enable_if<is_same<T, FirebaseJson>::value, FirebaseJson &>::type
  {
    return *to<FirebaseJson *>();
  }

  template <typename T>
  auto to() -> typename enable_if<is_same<T, FirebaseJsonArray *>::value, FirebaseJsonArray *>::type
  {
    if (!session.arrPtr)
      session.arrPtr = new FirebaseJsonArray();

    if (session.rtdb.resp_data_type == d_array)
    {
      if (session.jsonPtr)
        session.jsonPtr->clear();
      session.arrPtr->clear();
      session.arrPtr->setJsonArrayData(session.rtdb.raw.c_str());
    }

    return session.arrPtr;
  }

  template <typename T>
  auto to() -> typename enable_if<is_same<T, FirebaseJsonArray>::value, FirebaseJsonArray &>::type
  {
    return *to<FirebaseJsonArray *>();
  }

  template <typename T>
  auto to() -> typename enable_if<is_same<T, MB_VECTOR<uint8_t> *>::value, MB_VECTOR<uint8_t> *>::type
  {
    return session.rtdb.blob;
  }

#if defined(MBFS_FLASH_FS) && (defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB))
  template <typename T>
  auto to() -> typename enable_if<is_same<T, fs::File>::value, fs::File>::type
  {
    if (session.rtdb.resp_data_type == firebase_data_type::d_file)
    {
      int ret = Core.mbfs.open(pgm2Str(firebase_rtdb_pgm_str_10 /* "/fb_bin_0.tmp" */),
                               mbfs_type mem_storage_type_flash, mb_fs_open_mode_read);
      if (ret < 0)
        session.response.code = ret;
    }

    return Core.mbfs.getFlashFile();
  }
#endif

#endif

  /** Return the new appended node's name or key of server returned payload when calling pushXXX function (RTDB only).
   *
   * @return String (String object).
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String pushName();
#endif

  /** Get the stream connection status (RTDB only).
   *
   * @return Boolean type status indicates whether the Firebase Data object is working with a stream or not.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool isStream();
#endif

  /** Get the server connection status.
   *
   * @return Boolean type status indicates whether the Firebase Data object is connected to the server or not.
   */
  bool httpConnected();

  /** Get the timeout event of the server's stream (30 sec is the default) (RTDB only).
   * Nothing to do when stream connection timeout, the stream connection will be automatically resumed.
   *
   * @return Boolean type status indicates whether the stream was a timeout or not.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool streamTimeout();
#endif

  /** Get the availability of data or payload returned from the server (RTDB only).
   *
   * @return Boolean type status indicates whether the server return the new payload or not.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool dataAvailable();
#endif

  /** Get the availability of stream event-data payload returned from the server (RTDB only).
   *
   * @return Boolean type status indicates whether the server returns the stream event-data
   * payload or not.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool streamAvailable();
#endif

  /** Get the matching between data type that intends to get from/store to database and the server's return payload data type (RTDB only).
   *
   * @return Boolean type status indicates whether the type of data being get from/store to database
   * and the server's returned payload is matched or not.
   *
   * @note Data type checking was disable by default, which can be enabled via the Firebase Config e.g.
   * config.rtdb.data_type_stricted = true
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  bool mismatchDataType();
#endif

  /** Get the HTTP status code returned from the server.
   *
   * @return integer number of HTTP status.
   */
  int httpCode();

  /** Get the HTTP payload length returned from the server.
   *
   * @return integer number of payload length.
   */
  int payloadLength();

  /** Get the maximum size of HTTP payload length returned from the server.
   *
   * @return integer number of max payload length.
   */
  int maxPayloadLength();

  /** Check the overflow of the returned payload data buffer (RTDB only).
   *
   * @return The overflow status.
   *
   * @note Total default HTTP response buffer size is 400 bytes which can be set through FirebaseData.setResponseSize.
   */
  bool bufferOverflow();

  /** Get the name (full path) of the backup file in SD card/Flash memory (RTDB only).
   *
   * @return String (String object) of the file name that stores on SD card/Flash memory after backup operation.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  String getBackupFilename();
#endif

  /** Get the size of the backup file.
   *
   * @return Size of backup file in byte after backup operation.
   */
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  size_t getBackupFileSize();
#endif

  /** Clear or empty data in Firebase Data object.
   */
  void clear();

  /** Get the error description for file transferring (push file, set file, backup and restore) (RTDB only).
   *
   * @return Error description string (String object).
   */
  String fileTransferError();

  /** Return the server's payload data.
   *
   * @return Payload string (String object).
   */
  String payload();

  /** Setup TCP KeepAlive for internal TCP client.
   *
   * @param tcpKeepIdleSeconds lwIP TCP Keepalive idle in seconds.
   * @param tcpKeepIntervalSeconds lwIP TCP Keepalive interval in seconds.
   * @param tcpKeepCount lwIP TCP Keepalive count.
   *
   * For the TCP (KeepAlive) options, see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/lwip.html#tcp-options.
   *
   * If value of one of these parameters is zero, the TCP KeepAlive will be disabled.
   *
   * You can check the server connecting status, by exexuting <FirebaseData>.httpConnected() which will return true when connection to the server is still alive.
   */
  void keepAlive(int tcpKeepIdleSeconds, int tcpKeepIntervalSeconds, int tcpKeepCount);

  /** Get TCP KeepAlive status.
   *
   * @return Boolean status of TCP Keepalive.
   */
  bool isKeepAlive();

  Firebase_TCP_Client tcpClient;

#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)
  FCMObject fcm;
#endif
#endif

private:
  BearSSL_Session bsslSession;
  FB_ResponseCallback _responseCallback = NULL;

  FB_NetworkConnectionRequestCallback _networkConnectionCB;
  FB_NetworkStatusRequestCallback _networkStatusCB;
  Client *_client = nullptr;

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  StreamEventCallback _dataAvailableCallback = NULL;
  MultiPathStreamEventCallback _multiPathDataCallback = NULL;
  StreamTimeoutCallback _timeoutCallback = NULL;
  QueueInfoCallback _queueInfoCallback = NULL;
#endif
#if defined(FIREBASE_ESP_CLIENT)
#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)
  FunctionsOperationCallback _functionsOperationCallback = NULL;
#endif
#endif

  bool intCfg = false;
  unsigned long last_reconnect_millis = 0;
  uint16_t reconnect_tmo = 10 * 1000;
  firebase_session_info sessionPtr;
  firebase_session_info queueSessionPtr;

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  QueueManager _qMan;
  union IVal
  {
    uint64_t uint64;
    int64_t int64;
    uint32_t uint32;
    int32_t int32;
    int16_t int16;
    uint16_t uint16;
    int8_t int8;
    uint8_t uint8;
  };

  struct FVal
  {
    double d = 0;
    float f = 0;
    void setd(double v)
    {
      d = v;
      f = static_cast<float>(v);
    }

    void setf(float v)
    {
      f = v;
      d = static_cast<double>(v);
    }
  };

  IVal iVal = {0};
  FVal fVal;
#endif
  struct firebase_session_info_t session;

  void closeSession();
  bool handleStreamRead();
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
  void createResumableTask(struct fb_gcs_upload_resumable_task_info_t &ruTask, size_t fileSize,
                           const MB_String &location, const MB_String &local, const MB_String &remote,
                           firebase_mem_storage_type type, firebase_gcs_request_type reqType);
#endif
  bool waitResponse(struct firebase_tcp_response_handler_t &tcpHandler);
  bool isConnected(unsigned long &dataTime);
  void waitRxReady();
  bool readPayload(MB_String *chunkOut, struct firebase_tcp_response_handler_t &tcpHandler,
                   struct server_response_data_t &response);
  bool readResponse(MB_String *payload, struct firebase_tcp_response_handler_t &tcpHandler,
                    struct server_response_data_t &response);
  bool prepareDownload(const MB_String &filename, firebase_mem_storage_type type, bool openFileInWrireMode = false);
  void prepareDownloadOTA(struct firebase_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response);
  void endDownloadOTA(struct firebase_tcp_response_handler_t &tcpHandler);
  bool processDownload(const MB_String &filename, firebase_mem_storage_type type, uint8_t *buf,
                       int bufLen, struct firebase_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response,
                       int &stage, bool isOTA);
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
  bool getUploadInfo(int type, int &stage, const MB_String &pChunk, bool isList, bool isMeta,
                     struct firebase_fcs_file_list_item_t *fileitem, int &pos);
  void getAllUploadInfo(int type, int &currentStage, const MB_String &payload, bool isList, bool isMeta,
                        struct firebase_fcs_file_list_item_t *fileitem);
#endif

  void getError(MB_String &payload, struct firebase_tcp_response_handler_t &tcpHandler,
                struct server_response_data_t &response, bool clearPayload);
  void clearJson();
  void freeJson();
  void initJson();
  void checkOvf(size_t len, struct server_response_data_t &resp);
  bool reconnect(unsigned long dataTime = 0);
  MB_String getDataType(uint8_t type);
  MB_String getMethod(uint8_t method);
  bool tokenReady();
  void setTimeout();
  void setSecure();
#if defined(ENABLE_ERROR_QUEUE) || defined(FIREBASE_ENABLE_ERROR_QUEUE) && (defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB))
  void addQueue(QueueItem *qItem);
#endif
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
  void clearQueueItem(QueueItem *item);
  void sendStreamToCB(int code, bool report = true);
  void mSetIntValue(const char *value);
  void mSetFloatValue(const char *value);
  void mSetBoolValue(bool value);
  template <typename T>
  void restoreValue(int addr)
  {
    T *ptr = addrTo<T *>(addr);
    if (ptr)
      *ptr = to<T>();
  }
  void restoreCString(int addr)
  {
    char *ptr = addrTo<char *>(addr);
    if (ptr)
    {
      strcpy(ptr, to<const char *>());
      ptr[strlen(to<const char *>())] = '\0';
    }
  }
#endif
  void addSession(firebase_con_mode mode);
  void setSession(bool remove, bool status);
  int tcpSend(const char *s);
  int tcpWrite(const uint8_t *data, size_t size);
  void addQueueSession();
  void removeQueueSession();
  void setRaw(bool trim);
  bool configReady()
  {
    if (!Core.config)
    {
      session.response.code = FIREBASE_ERROR_UNINITIALIZED;
      return false;
    }
    return true;
  }
};

#endif
