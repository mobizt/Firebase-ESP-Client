
/**
 * Google's Firebase Cloud Messaging class, FCM.h version 1.1.0
 *
 * Created September 5, 2023
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

#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)

#ifndef FIREBASE_FCM_H
#define FIREBASE_FCM_H
#include "./FB_Utils.h"
#include "./session/FB_Session.h"

using namespace mb_string;

class FB_CM
{
  friend class Firebase_ESP_Client;

public:
  FB_CM();
  ~FB_CM();

  /** Set the server key.
   *
   * @param serverKey Server key found on Console: Project settings > Cloud Messaging
   * @param spi_ethernet_module SPI_ETH_Module struct data, optional for ESP8266 use with Ethernet module.
   *
   * @note This server key required for sending message via legacy HTTP API.
   *
   * SPI_ETH_Module struct data is for ESP8266 Ethernet supported module lwip interface.
   * The usage example for Ethernet.
   *
   * ////////////////////
   * #include <ENC28J60lwIP.h>
   *
   * #define ETH_CS_PIN 16 //GPIO 16 connected to Ethernet module (ENC28J60) CS pin
   *
   * ENC28J60lwIP eth(ETH_CS_PIN);
   *
   * FirebaseData fbdo;
   *
   * SPI_ETH_Module spi_ethernet_module;
   *
   * spi_ethernet_module.enc28j60 = &eth;
   * fbdoFirebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY, &spi_ethernet_module);
   *
   * ////////////////////
   * The API key created in the Google Cloud console, cannot be used for authorizing FCM requests.
   */
  template <typename T = const char *>
  void setServerKey(T serverKey, SPI_ETH_Module *spi_ethernet_module = NULL)
  {
    mSetServerKey(toStringPtr(serverKey), spi_ethernet_module);
  }

  /** Clear all WiFi access points assigned.
   *
   */
  void clearAP()
  {
    Core.wifiCreds.clearAP();
  }

  /** Add WiFi access point for non-ESP device to resume WiFi connection.
   *
   * @param ssid The WiFi SSID.
   * @param password The WiFi password.
   */
  void addAP(const String &ssid, const String &password)
  {
    Core.wifiCreds.addAP(ssid, password);
  }

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
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = size_t>
  bool subscribeTopic(FirebaseData *fbdo, T1 topic, T2 IID[], T3 numToken)
  {
    return mSubscribeTopic(fbdo, toStringPtr(topic), IID, numToken);
  }

  /** Unsubscribe the devices from the topic.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param topic The topic to romove the subscription.
   * @param IID The instance ID tokens or registration tokens array.
   * @param numToken The size of instance ID tokens array.
   * @return Boolean type status indicates the success of the operation.
   *
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = size_t>
  bool unsubscribeTopic(FirebaseData *fbdo, T1 topic, T2 IID[], T3 numToken)
  {
    return mUnsubscribeTopic(fbdo, toStringPtr(topic), IID, numToken);
  }

  /** Get the app instance info.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param IID The instance ID token of device.
   * @return Boolean type status indicates the success of the operation.
   *
   */
  template <typename T = const char *>
  bool appInstanceInfo(FirebaseData *fbdo, T IID) { return mAppInstanceInfo(fbdo, IID); }

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
  template <typename T1 = const char *, typename T2 = const char **, typename T3 = size_t>
  bool regisAPNsTokens(FirebaseData *fbdo, T1 application, bool sandbox, T2 APNs[], T3 numToken)
  {
    return mRegisAPNsTokens(fbdo, toStringPtr(application), sandbox, APNs, numToken);
  }

  /** Get the server payload.
   *
   * @param fbdo The pointer to Firebase Data Object..
   * @return String of payload returned from the server.
   */
  String payload(FirebaseData *fbdo);

private:
  bool handleFCMRequest(FirebaseData *fbdo, firebase_fcm_msg_mode mode, const char *payload);
  bool waitResponse(FirebaseData *fbdo);
  bool handleResponse(FirebaseData *fbdo);
  void rescon(FirebaseData *fbdo, const char *host);
  void fcm_connect(FirebaseData *fbdo, firebase_fcm_msg_mode mode);
  bool fcm_send(FirebaseData *fbdo, firebase_fcm_msg_mode mode, const char *msg);
  bool sendHeader(FirebaseData *fbdo, firebase_fcm_msg_mode mode, const char *payload);
  void fcm_prepareLegacyPayload(FCM_Legacy_HTTP_Message *msg);
  void fcm_prepareV1Payload(FCM_HTTPv1_JSON_Message *msg);
  void fcm_preparSubscriptionPayload(const char *topic, const char *IID[], size_t numToken);
  void fcm_preparAPNsRegistPayload(const char *application, bool sandbox, const char *APNs[], size_t numToken);

  void mSetServerKey(MB_StringPtr serverKey, SPI_ETH_Module *spi_ethernet_module = NULL);
  bool checkServerKey(FirebaseData *fbdo);
  bool mSubscribeTopic(FirebaseData *fbdo, MB_StringPtr topic, const char *IID[], size_t numToken);
  bool mUnsubscribeTopic(FirebaseData *fbdo, MB_StringPtr topic, const char *IID[], size_t numToken);
  bool mAppInstanceInfo(FirebaseData *fbdo, const char *IID);
  bool mRegisAPNsTokens(FirebaseData *fbdo, MB_StringPtr application, bool sandbox, const char *APNs[], size_t numToken);
  void clear();

  MB_String server_key;
  MB_String raw;
  uint16_t port = FIREBASE_PORT;
  SPI_ETH_Module *_spi_ethernet_module = NULL;
};

#endif

#endif // ENABLE