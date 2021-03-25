/**
 * Google's Firebase Cloud Messaging class, FCM.h version 1.0.4
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

#ifndef FIREBASE_FCM_H
#define FIREBASE_FCM_H
#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"

class FB_CM
{
  friend class Firebase_ESP_Client;

public:
  FB_CM();
  ~FB_CM();

  /** Set the server key.
   * 
   * @param serverKey Server key found on Console: Project settings > Cloud Messaging
   * 
   * @note This server key required for sending message via legacy HTTP API.
   * 
   * The API key created in the Google Cloud console, cannot be used for authorizing FCM requests. 
   */
  void setServerKey(const char *serverKey);

  /** Send Firebase Cloud Messaging to the devices with JSON payload using the FCM legacy API.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param msg The pointer to the message to send which is the FCM_Legacy_JSON_Message type data.
   * @return Boolean type status indicates the success of the operation.
   * 
   * @note The FCM_Legacy_JSON_Message properties are
   * targets - The targets of messages e.g. to, registration_ids, condition.
   * 
   * options - The options of message contained the sub-properties e.g, collapse_key, priority, content_available, 
   * mutable_content,time_to_live, restricted_package_name, and dry_run.
   * The sub-properties value of the options should be assigned in string.
   * 
   * payloads - The two payloads i.e. notification and data.
   * 
   * The payloads.notification properties are available e.g. 
   * title  string  all
   * body string  all
   * icon string  Andoid, web
   * click_action string  all
   * sound  string  iOS, Android
   * badge number iOS
   * subtitle string  iOS
   * body_loc_key string  iOS, Android
   * body_loc_args  JSON array of string  iOS, Android
   * title_loc_key  string  iOS, Android
   * title_loc_args JSON array of string  iOS, Android
   * android_channel_id string  Android
   * tag  string  Android
   * color  string  Android
   * 
   * The payloads.data is the JSON object. 
   * 
   * Read more details about legacy HTTP API here https://firebase.google.com/docs/cloud-messaging/http-server-ref
  */
  bool send(FirebaseData *fbdo, FCM_Legacy_HTTP_Message *msg);

  /** Send Firebase Cloud Messaging to the devices using the FCM HTTP v1 API.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param msg The pointer to the message to send which is the FCM_HTTPv1_JSON_Message type data.
   * @return Boolean type status indicates the success of the operation.
   * 
   * Read more details about HTTP v1 API here https://firebase.google.com/docs/reference/fcm/rest/v1/projects.messages
  */
  bool send(FirebaseData *fbdo, FCM_HTTPv1_JSON_Message *msg);

  /** Subscribe the devices to the topic.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param topic The topic to subscribe.
   * @param IID The instance ID tokens or registration tokens array.
   * @param numToken The size of instance ID tokens array.
   * @return Boolean type status indicates the success of the operation.
   * 
  */
  bool subscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], size_t numToken);

  /** Unsubscribe the devices from the topic.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param topic The topic to romove the subscription.
   * @param IID The instance ID tokens or registration tokens array.
   * @param numToken The size of instance ID tokens array.
   * @return Boolean type status indicates the success of the operation.
   * 
  */
  bool unsubscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], size_t numToken);

  /** Get the app instance info.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param IID The instance ID token of device.
   * @return Boolean type status indicates the success of the operation.
   * 
  */
  bool appInstanceInfo(FirebaseData *fbdo, const char *IID);

  /** Create registration tokens for APNs tokens.
   * 
   * @param fbdo The pointer to Firebase Data Object.
   * @param application The Bundle id of the app.
   * @param sandbox The Boolean to indicate sandbox environment (TRUE) or production (FALSE).
   * @param APNs The iOS APNs tokens array.
   * @param numToken The size of instance ID tokens array.
   * @return Boolean type status indicates the success of the operation.
   * 
  */
  bool regisAPNsTokens(FirebaseData *fbdo, const char *application, bool sandbox, const char *APNs[], size_t numToken);

  /** Get the server payload.
   * 
   * @param fbdo The pointer to Firebase Data Object..
   * @return String of payload returned from the server.
  */
  String payload(FirebaseData *fbdo);

private:
  bool init(bool clearInt= false);
  void begin(UtilsClass *u);
  bool handleFCMRequest(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, std::string &payload);
  bool waitResponse(FirebaseData *fbdo);
  bool handleResponse(FirebaseData *fbdo);
  void rescon(FirebaseData *fbdo, const char *host);
  void fcm_connect(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode);
  bool fcm_send(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, std::string &msg);
  void fcm_prepareHeader(std::string &header, fb_esp_fcm_msg_mode mode, std::string &payload);
  void fcm_prepareLegacyPayload(std::string &buf, FCM_Legacy_HTTP_Message *msg);
  void fcm_prepareV1Payload(std::string &buf, FCM_HTTPv1_JSON_Message *msg);
  void fcm_preparSubscriptionPayload(std::string &buf, const char *topic, const char *IID[], size_t numToken);
  void fcm_preparAPNsRegistPayload(std::string &buf, const char *application, bool sandbox, const char *APNs[], size_t numToken);

  void clear();
  FirebaseConfig *_cfg = nullptr;
  FirebaseAuth *_auth = nullptr;
  UtilsClass *_ut = nullptr;
  std::string _topic = "";
  std::string _server_key = "";
  std::string _token = "";
  int _ttl = -1;
  uint16_t _index = 0;
  uint16_t _port = FIREBASE_PORT;
  FirebaseJson _fcmPayload;
  std::vector<std::string> _tokens = std::vector<std::string>();
  UtilsClass *ut = nullptr;
};

#endif