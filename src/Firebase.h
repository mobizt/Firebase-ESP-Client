#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * The Firebase class, Firebase.h v1.2.6
 *
 *  Created April 5, 2023
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

#ifndef Firebase_H
#define Firebase_H

#include <Arduino.h>
#include "mbfs/MB_MCU.h"

#include "FirebaseFS.h"
#include "FB_Const.h"

#if !defined(ESP32) && !defined(ESP8266) && !defined(MB_ARDUINO_PICO)
#ifndef FB_ENABLE_EXTERNAL_CLIENT
#define FB_ENABLE_EXTERNAL_CLIENT
#endif
#endif

#if defined(ESP8266) || defined(ESP32) || defined(FB_ENABLE_EXTERNAL_CLIENT) || defined(MB_ARDUINO_PICO)

#if !defined(ESP32) && !defined(ESP8266) && !defined(MB_ARDUINO_PICO)
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char *sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif // __arm__
#endif

#if defined(ESP8266)
#include <SPI.h>
#include <time.h>
#include <vector>
#include <functional>
#include <Schedule.h>
#include <ets_sys.h>
#endif

#include "FB_Utils.h"
#include "wcs/FB_Clients.h"
#include "signer/Signer.h"
#include "session/FB_Session.h"

#if defined(DEFAULT_SD_FS) && defined(CARD_TYPE_SD) && defined(ESP32) && defined(SD_FAT_VERSION)
class SdSpiConfig;
#endif

#if defined(FIREBASE_ESP_CLIENT)

#ifdef ENABLE_RTDB
#include "rtdb/FB_RTDB.h"
#endif
#ifdef ENABLE_FCM
#include "message/FCM.h"
#endif
#include "FB_Utils.h"
#ifdef ENABLE_FB_STORAGE
#include "storage/FCS.h"
#endif
#ifdef ENABLE_GC_STORAGE
#include "gcs/GCS.h"
#endif
#ifdef ENABLE_FIRESTORE
#include "firestore/FB_Firestore.h"
#endif
#ifdef ENABLE_FB_FUNCTIONS
#include "functions/FB_Functions.h"
#include "functions/FunctionsConfig.h"
#endif

#ifndef FPSTR
#define FPSTR MBSTRING_FLASH_MCR
#endif

class FIREBASE_CLASS
{
  friend class QueryFilter;
  friend class FirebaseSession;

public:
#ifdef ENABLE_RTDB
  FB_RTDB RTDB;
#endif
#ifdef ENABLE_FCM
  FB_CM FCM;
#endif
#ifdef ENABLE_FB_STORAGE
  FB_Storage Storage;
#endif
#ifdef ENABLE_FIRESTORE
  FB_Firestore Firestore;
#endif
#ifdef ENABLE_FB_FUNCTIONS
  FB_Functions Functions;
#endif
#ifdef ENABLE_GC_STORAGE
  GG_CloudStorage GCStorage;
#endif

  FIREBASE_CLASS();
  ~FIREBASE_CLASS();

  /** Initialize Firebase with the config and Firebase's authentication credentials.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param auth The pointer to FirebaseAuth data.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  void begin(FirebaseConfig *config, FirebaseAuth *auth);

  /** Setup the ID token for authentication.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param idToken The ID Token.
   * @param expire The expired interval in seeconds (max.3600 sec).
   * @param refreshToken The refresh token for token refreshment.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  void setIdToken(FirebaseConfig *config, T1 idToken, size_t expire = 3600, T2 refreshToken = "")
  {
    return mSetAuthToken(config, toStringPtr(idToken), expire, toStringPtr(refreshToken),
                         token_type_id_token, toStringPtr(""), toStringPtr(""));
  }

  /** Setup the access token for authentication.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param accessToken The access Token.
   * @param expire The expired interval in seeconds (max.3600 sec).
   * @param refreshToken The refresh token for token refreshment.
   * @param clientId The The client identifier issued to the client during the registration process.
   * @param clientSecret The client secret.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *>
  void setAccessToken(FirebaseConfig *config, T1 accessToken, size_t expire = 3600, T2 refreshToken = "",
                      T3 clientId = "", T4 clientSecret = "")
  {
    return mSetAuthToken(config, toStringPtr(accessToken), expire,
                         toStringPtr(refreshToken), token_type_oauth2_access_token,
                         toStringPtr(clientId), toStringPtr(clientSecret));
  }

  /** Setup the custom token for authentication.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param customToken The Identity Platform custom token.
   *
   * If the refresh token from Custom token verification or sign in, was assigned here instead of
   * custom token (signed JWT token), the token refresh process will be performed immediately.
   *
   * Any token that is not in the form header.payload.signature i.e., xxxxx.yyyyy.zzzzz will be treated as refresh token.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  template <typename T1 = const char *>
  void setCustomToken(FirebaseConfig *config, T1 customToken)
  {
    return mSetAuthToken(config, toStringPtr(customToken), 0, toStringPtr(""),
                         token_type_custom_token, toStringPtr(""), toStringPtr(""));
  }

  /** Check for token expiry status.
   *
   * @return bool of expiry status.
   */
  bool isTokenExpired();

  /** Force the token to expire immediately and refresh.
   *
   * @param config The pointer to FirebaseConfig data.
   */
  void refreshToken(FirebaseConfig *config);

  /** Reset stored config and auth credentials.
   *
   * @param config The pointer to FirebaseConfig data.
   *
   */
  void reset(FirebaseConfig *config);

  /** Provide the details of token generation.
   *
   * @return token_info_t The token_info_t structured data that indicates the status.
   *
   *  @note Use type property to get the type enum value.
   * token_type_undefined or 0,
   * token_type_legacy_token or 1,
   * token_type_id_token or 2,
   * token_type_custom_token or 3,
   * token_type_oauth2_access_token or 4
   *
   * Use status property to get the status enum value.
   * token_status_uninitialized or 0,
   * token_status_on_signing or 1,
   * token_status_on_request or 2,
   * token_status_on_refresh or 3,
   * token_status_ready or 4
   *
   * In case of token generation and refreshment errors,
   * use error.code property to get the error code number.
   * Use error.message property to get the error message string.
   *
   */
  struct token_info_t authTokenInfo();

  /** Provide the ready status of token generation.
   *
   * This function should be called repeatedly to handle authentication tasks.
   *
   * @return Boolean type status indicates the token generation is completed.
   */
  bool ready();

  /** Provide the grant access status for Firebase Services.
   *
   * @return Boolean type status indicates the device can access to the services
   *
   * This returns false if ready() returns false (token generation is not ready).
   */
  bool authenticated();

  /** Sign up for a new user.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param auth The pointer to FirebaseAuth data.
   * @param email The user Email.
   * @param password The user password.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note By calling Firebase.begin with config and auth after sign up will be signed in.
   *
   * This required Email/Password provider to be enabled,
   * From Firebase console, select Authentication, select Sign-in method tab,
   * under the Sign-in providers list, enable Email/Password provider.
   *
   * If the assigned email and passowrd are empty,
   * the anonymous user will be created if Anonymous provider is enabled.
   *
   * To enable Anonymous provider,
   * from Firebase console, select Authentication, select Sign-in method tab,
   * under the Sign-in providers list, enable Anonymous provider.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool signUp(FirebaseConfig *config, FirebaseAuth *auth, T1 email, T2 password)
  {
    return mSignUp(config, auth, toStringPtr(email), toStringPtr(password));
  }

  /** Delete user from project.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param auth The pointer to FirebaseAuth data.
   * @param idToken (optional) The id token of user, leave blank to delete the current sign in user.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool deleteUser(FirebaseConfig *config, FirebaseAuth *auth, T idToken = "")
  {
    return mDeleteUser(config, auth, toStringPtr(idToken));
  }

  /** Send a user a verification Email.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param idToken The id token of user that was already signed in with Email and password (optional).
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The id token can be obtained from Firebase.getToken()
   * after begin with config and auth data.
   *
   * If the idToken is not assigned, the internal id_token will be used.
   *
   * See the Templates of Email address verification in the Firebase console
   * , Authentication.
   */
  template <typename T = const char *>
  bool sendEmailVerification(FirebaseConfig *config, T idToken = "")
  {
    return msendEmailVerification(config, toStringPtr(idToken));
  }

  /** Send a user a password reset link to Email.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param email The user Email to send the password resset link.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool sendResetPassword(FirebaseConfig *config, T email) { return mSendResetPassword(config, toStringPtr(email)); }

  /** Reconnect WiFi if lost connection.
   *
   * @param reconnect The boolean to set/unset WiFi AP reconnection.
   */
  void reconnectWiFi(bool reconnect);

  /** Assign UDP client and gmt offset for NTP time synching when using external SSL client
   * @param client The pointer to UDP client based on the network type.
   * @param gmtOffset The GMT time offset.
   */
  void setUDPClient(UDP *client, float gmtOffset);

  /** Get currently used auth token string.
   *
   * @return constant char* of currently used auth token.
   */
  const char *getToken();

  /** Get refresh token string.
   *
   * @return constant char* of refresh token.
   */
  const char *getRefreshToken();

  /** Get free Heap memory.
   *
   * @return free Heap memory size.
   */
  int getFreeHeap();

  /** Get current timestamp.
   *
   * @return current timestamp.
   */
  time_t getCurrentTime();

  /** Set the decimal places for float value to be stored in database.
   *
   * @param digits The decimal places.
   */
  void setFloatDigits(uint8_t digits);

  /** Set the decimal places for double value to be stored in database.
   *
   * @param digits The decimal places.
   */
  void setDoubleDigits(uint8_t digits);

#if defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)

  /** Initiate SD card with SPI port configuration.
   *
   * @param ss SPI Chip/Slave Select pin.
   * @param sck SPI Clock pin.
   * @param miso SPI MISO pin.
   * @param mosi SPI MOSI pin.
   * @param frequency The SPI frequency
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, uint32_t frequency = 4000000);

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)

  /** Initiate SD card with SD FS configurations (ESP8266 only).
   *
   * @param ss SPI Chip/Slave Select pin.
   * @param sdFSConfig The pointer to SDFSConfig object (ESP8266 and Pico only).
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(SDFSConfig *sdFSConfig);

#endif

#if defined(ESP32)
  /** Initiate SD card with chip select and SPI configuration (ESP32 only).
   *
   * @param ss SPI Chip/Slave Select pin.
   * @param spiConfig The pointer to SPIClass object for SPI configuartion.
   * @param frequency The SPI frequency.
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(int8_t ss, SPIClass *spiConfig = nullptr, uint32_t frequency = 4000000);
#endif

#if defined(MBFS_ESP32_SDFAT_ENABLED) || defined(MBFS_SDFAT_ENABLED)
  /** Initiate SD card with SdFat SPI and pins configurations (with SdFat included only).
   *
   * @param sdFatSPIConfig The pointer to SdSpiConfig object for SdFat SPI configuration.
   * @param ss SPI Chip/Slave Select pin.
   * @param sck SPI Clock pin.
   * @param miso SPI MISO pin.
   * @param mosi SPI MOSI pin.
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);

  /** Initiate SD card with SdFat SDIO configuration (with SdFat included only).
   *
   * @param sdFatSDIOConfig The pointer to SdioConfig object for SdFat SDIO configuration.
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(SdioConfig *sdFatSDIOConfig);

#endif

#endif

#if defined(ESP32) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD_MMC)
  /** Initialize the SD_MMC card (ESP32 only).
   *
   * @param mountpoint The mounting point.
   * @param mode1bit Allow 1 bit data line (SPI mode).
   * @param format_if_mount_failed Format SD_MMC card if mount failed.
   * @return The boolean value indicates the success of operation.
   */
  bool sdMMCBegin(const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false);
#endif

  /** Set system time with timestamp.
   *
   * @param ts timestamp in seconds from midnight Jan 1, 1970.
   * @return Boolean type status indicates the success of the operation.
   */
  bool setSystemTime(time_t ts);

  /** Provide the http code error string
   *
   * @param httpCode The http code.
   * @param buff The String buffer out.
   */
  void errorToString(int httpCode, String &buff)
  {
    MB_String out;
    Signer.errorToString(httpCode, out);
    buff = out.c_str();
  }

private:
  void init(FirebaseConfig *config, FirebaseAuth *auth);
  void mSetAuthToken(FirebaseConfig *config, MB_StringPtr authToken, size_t expire, MB_StringPtr refreshToken,
                     fb_esp_auth_token_type type, MB_StringPtr clientId, MB_StringPtr clientSecret);
  bool mSignUp(FirebaseConfig *config, FirebaseAuth *auth, MB_StringPtr email, MB_StringPtr password);
  bool msendEmailVerification(FirebaseConfig *config, MB_StringPtr idToken);
  bool mDeleteUser(FirebaseConfig *config, FirebaseAuth *auth, MB_StringPtr idToken);
  bool mSendResetPassword(FirebaseConfig *config, MB_StringPtr email);

  FirebaseAuth *auth = nullptr;
  FirebaseConfig *config = nullptr;
  MB_FS mbfs;
  uint32_t mb_ts = 0;
  uint32_t mb_ts_offset = 0;
};

extern FIREBASE_CLASS Firebase;

#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)

#ifndef FPSTR
#define FPSTR MBSTRING_FLASH_MCR
#endif

#ifdef ENABLE_RTDB
#include "rtdb/FB_RTDB.h"
#endif

class FIREBASE_CLASS
{
  friend class QueryFilter;
  friend class FirebaseSession;

public:
#ifdef ENABLE_RTDB
  FB_RTDB RTDB;
#endif

  FIREBASE_CLASS();
  ~FIREBASE_CLASS();

  /** Initialize Firebase with the config and Firebase's authentication credentials.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param auth The pointer to FirebaseAuth data.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  void begin(FirebaseConfig *config, FirebaseAuth *auth);

  /** Setup the ID token for authentication.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param idToken The ID Token.
   * @param expire The expired interval in seeconds (max.3600 sec).
   * @param refreshToken The refresh token for token refreshment.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  void setIdToken(FirebaseConfig *config, T1 idToken, size_t expire = 3600, T2 refreshToken = "")
  {
    return mSetAuthToken(config, toStringPtr(idToken), expire, toStringPtr(refreshToken),
                         token_type_id_token, toStringPtr(""), toStringPtr(""));
  }

  /** Setup the access token for authentication.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param accessToken The access Token.
   * @param expire The expired interval in seeconds (max.3600 sec).
   * @param refreshToken The refresh token for token refreshment.
   * @param clientId The The client identifier issued to the client during the registration process.
   * @param clientSecret The client secret.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *>
  void setAccessToken(FirebaseConfig *config, T1 accessToken, size_t expire = 3600,
                      T2 refreshToken = "", T3 clientId = "", T4 clientSecret = "")
  {
    return mSetAuthToken(config, toStringPtr(accessToken), expire, toStringPtr(refreshToken),
                         token_type_oauth2_access_token, toStringPtr(clientId), toStringPtr(clientSecret));
  }

  /** Setup the custom token for authentication.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param customToken The Identity Platform custom token.
   *
   * If the refresh token from Custom token verification or sign in, was assigned here instead of
   * custom token (signed JWT token), the token refresh process will be performed immediately.
   *
   * Any token that is not in the form header.payload.signature i.e., xxxxx.yyyyy.zzzzz will be treated as refresh token.
   *
   * @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
   */
  template <typename T1 = const char *>
  void setCustomToken(FirebaseConfig *config, T1 customToken)
  {
    return mSetAuthToken(config, toStringPtr(customToken), 0, toStringPtr(""),
                         token_type_custom_token, toStringPtr(""), toStringPtr(""));
  }

  /** Check for token expiry status.
   *
   * @return bool of expiry status.
   */
  bool isTokenExpired();

  /** Force the token to expire immediately and refresh.
   *
   * @param config The pointer to FirebaseConfig data.
   */
  void refreshToken(FirebaseConfig *config);

  /** Reset stored config authentiocation credentials.
   *
   * @param config The pointer to FirebaseConfig data.
   *
   */
  void reset(FirebaseConfig *config);

  /** Provide the details of token generation.
   *
   * @return token_info_t The token_info_t structured data that indicates the status.
   *
   * @note Use type property to get the type enum value.
   * token_type_undefined or 0,
   * token_type_legacy_token or 1,
   * token_type_id_token or 2,
   * token_type_custom_token or 3,
   * token_type_oauth2_access_token or 4
   *
   * Use status property to get the status enum value.
   * token_status_uninitialized or 0,
   * token_status_on_signing or 1,
   * token_status_on_request or 2,
   * token_status_on_refresh or 3,
   * token_status_ready or 4
   *
   * In case of token generation and refreshment errors,
   * use error.code property to get the error code number.
   * Use error.message property to get the error message string.
   *
   */
  struct token_info_t authTokenInfo();

  /** Provide the ready status of token generation.
   *
   * This function should be called repeatedly to handle authentication tasks.
   *
   * @return Boolean type status indicates the token generation is completed.
   */
  bool ready();

  /** Provide the grant access status for Firebase Services.
   *
   * @return Boolean type status indicates the device can access to the services
   *
   * This returns false if ready() returns false (token generation is not ready).
   */
  bool authenticated();

  /** Store Firebase's legacy authentication credentials.
   *
   * @param databaseURL Your RTDB URL e.g. <databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
   * @param databaseSecret Your database secret.
   * @param caCert Root CA certificate base64 string (PEM file).
   * @param caCertFile Root CA certificate DER file (binary).
   * @param StorageType Type of storage, StorageType::SD and StorageType::FLASH.
   * @param GMTOffset Optional for ESP8266. GMT time offset in hour is required to
   * set the time for BearSSL certificate verification.
   *
   * @note This parameter is only required for ESP8266 Core SDK v2.5.x or later.
   * The Root CA certificate DER file is only supported in Core SDK v2.5.x
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  void begin(T1 databaseURL, T2 databaseSecret)
  {
    pre_begin(toStringPtr(databaseURL), toStringPtr(databaseSecret));
    begin(config, auth);
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  void begin(T1 databaseURL, T2 databaseSecret, T3 caCert, float GMTOffset = 0.0)
  {
    pre_begin(toStringPtr(databaseURL), toStringPtr(databaseSecret));
    if (caCert)
    {
      float _gmtOffset = GMTOffset;
      config->cert.data = caCert;
#ifdef ESP8266
      if (GMTOffset >= -12.0 && GMTOffset <= 14.0)
        _gmtOffset = GMTOffset;
      TimeHelper::syncClock(&Signer.ntpClient, mb_ts, mb_ts_offset, _gmtOffset, config);
#endif
    }
    begin(config, auth);
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  void begin(T1 databaseURL, T2 databaseSecret, T3 caCertFile, uint8_t storageType, float GMTOffset = 0.0)
  {
    pre_begin(toStringPtr(databaseURL), toStringPtr(databaseSecret));

    MB_String _caCertFile = caCertFile;

    if (_caCertFile.length() > 0)
    {
      float _gmtOffset = GMTOffset;
      config->cert.file = _caCertFile;
      config->cert.file_storage = storageType;
#ifdef ESP8266
      if (GMTOffset >= -12.0 && GMTOffset <= 14.0)
        _gmtOffset = GMTOffset;
      TimeHelper::syncClock(&Signer.ntpClient, mb_ts, mb_ts_offset, _gmtOffset, config);
#endif
    }
    begin(config, auth);
  }
  /** Stop Firebase and release all resources.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   */
  void end(FirebaseData &fbdo);

  /** Sign up for a new user.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param auth The pointer to FirebaseAuth data.
   * @param email The user Email.
   * @param password The user password.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note By calling Firebase.begin with config and auth after sign up will be signed in.
   *
   * This required Email/Password provider to be enabled,
   * From Firebase console, select Authentication, select Sign-in method tab,
   * under the Sign-in providers list, enable Email/Password provider.
   *
   * If the assigned email and passowrd are empty,
   * the anonymous user will be created if Anonymous provider is enabled.
   *
   * To enable Anonymous provider,
   * from Firebase console, select Authentication, select Sign-in method tab,
   * under the Sign-in providers list, enable Anonymous provider.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool signUp(FirebaseConfig *config, FirebaseAuth *auth, T1 email, T2 password)
  {
    return mSignUp(config, auth, toStringPtr(email), toStringPtr(password));
  }

  /** Delete user from project.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param auth The pointer to FirebaseAuth data.
   * @param idToken (optional) The id token of user, leave blank to delete the current sign in user.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool deleteUser(FirebaseConfig *config, FirebaseAuth *auth, T idToken = "")
  {
    return mDeleteUser(config, auth, toStringPtr(idToken));
  }

  /** Send a user a verification Email.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param idToken The id token of user that was already signed in with Email and password (optional).
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The id token can be obtained from config.signer.tokens.id_token
   * after begin with config and auth data
   *
   * If the idToken is not assigned, the internal config.signer.tokens.id_token
   * will be used.
   *
   * See the Templates of Email address verification in the Firebase console
   * , Authentication.
   *
   */
  template <typename T = const char *>
  bool sendEmailVerification(FirebaseConfig *config, T idToken = "")
  {
    return msendEmailVerification(config, toStringPtr(idToken));
  }

  /** Send a user a password reset link to Email.
   *
   * @param config The pointer to FirebaseConfig data.
   * @param email The user Email to send the password resset link.
   * @return Boolean type status indicates the success of the operation.
   *
   */
  template <typename T = const char *>
  bool sendResetPassword(FirebaseConfig *config, T email)
  {
    return mSendResetPassword(config, toStringPtr(email));
  }

  /** Reconnect WiFi if lost connection.
   *
   * @param reconnect The boolean to set/unset WiFi AP reconnection.
   */
  void reconnectWiFi(bool reconnect);

  /** Assign UDP client and gmt offset for NTP time synching when using external SSL client
   * @param client The pointer to UDP client based on the network type.
   * @param gmtOffset The GMT time offset.
   */
  void setUDPClient(UDP *client, float gmtOffset);

  /** Get currently used auth token string.
   *
   * @return constant char* of currently used auth token.
   */
  const char *getToken();

  /** Get free Heap memory.
   *
   * @return free Heap memory size.
   */
  int getFreeHeap();

  /** Get current timestamp.
   *
   * @return current timestamp.
   */
  time_t getCurrentTime();

  /** Set the decimal places for float value to be stored in database.
   *
   * @param digits The decimal places.
   */
  void setFloatDigits(uint8_t digits);

  /** Set the decimal places for double value to be stored in database.
   *
   * @param digits The decimal places.
   */
  void setDoubleDigits(uint8_t digits);

#ifdef ENABLE_RTDB

#ifdef ESP32
  /** Enable multiple HTTP requests at a time.
   *
   * @param enable - The boolean value to enable/disable.
   *
   * @note The multiple HTTP requessts at a time is disable by default to prevent the large memory used in multiple requests.
   */
  void allowMultipleRequests(bool enable)
  {
    if (Signer.config)
      Signer.config->internal.fb_multiple_requests = enable;
  }
#endif

  /** Set the timeout of Firebase.get functions.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param millisec The milliseconds to limit the request (0 900,000 ms or 15 min).
   */
  void setReadTimeout(FirebaseData &fbdo, int millisec) { RTDB.setReadTimeout(&fbdo, millisec); }

  /** Set the size limit of payload data that will write to the database for each request.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param size The size identified string e.g. tiny, small, medium, large and unlimited.
   *
   * @note Size string and its write timeout in seconds e.g. tiny (1s), small (10s), medium (30s) and large (60s).
   */
  template <typename T = const char *>
  void setwriteSizeLimit(FirebaseData &fbdo, T size) { return RTDB.setwriteSizeLimit(&fbdo, size); }

  /** Read the database rules.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].jsonData will return the JSON string value of
   * database rules returned from the server.
   */
  bool getRules(FirebaseData &fbdo) { return RTDB.getRules(&fbdo); }

  /** Save the database rules to file.
   *
   * @param fbdo The pointer to Firebase Data Object.
   * @param storageType Type of storage to read file data, StorageType::FLASH or StorageType::SD.
   * @param filename Filename to save rules.
   * @param callback Optional. The callback function that accept RTDB_DownloadStatusInfo data.
   * @return Boolean value, indicates the success of the operation.
   *
   */
  template <typename T = const char *>
  bool getRules(FirebaseData &fbdo, uint8_t storageType, T filename, RTDB_DownloadProgressCallback callback = NULL)
  {
    return RTDB.getRules(&fbdo, getMemStorageType(storageType), filename, callback);
  }

  /** Write the database rules.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param rules Database rules in JSON String format.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool setRules(FirebaseData &fbdo, T rules) { return RTDB.setRules(&fbdo, rules); }

  /** Write the database rules from file.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param storageType Type of storage to read file data, StorageType::FLASH or StorageType::SD.
   * @param filename Filename to read the rules from.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool setRules(FirebaseData &fbdo, uint8_t storageType, T filename, RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.setRules(&fbdo, getMemStorageType(storageType), filename, callback);
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
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *, typename T5 = const char *>
  bool setReadWriteRules(FirebaseData &fbdo, T1 path, T2 var, T3 readVal, T4 writeVal, T5 databaseSecret)
  {
    return RTDB.setReadWriteRules(&fbdo, path, var, readVal, writeVal, databaseSecret);
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
  bool setQueryIndex(FirebaseData &fbdo, T1 path, T2 node, T3 databaseSecret)
  {
    return RTDB.setQueryIndex(&fbdo, path, node, databaseSecret);
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
  bool removeQueryIndex(FirebaseData &fbdo, T1 path, T2 databaseSecret)
  {
    return RTDB.removeQueryIndex(&fbdo, path, databaseSecret);
  }

  /** Determine whether the defined database path exists or not.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path to be checked.
   * @return Boolean type result indicates whether the defined path existed or not.
   */
  template <typename T = const char *>
  bool pathExisted(FirebaseData &fbdo, T path) { return RTDB.pathExisted(&fbdo, path); }

  /** Deprecated */
  template <typename T = const char *>
  bool pathExist(FirebaseData &fbdo, T path) { return RTDB.pathExisted(&fbdo, path); }

  /** Determine the unique identifier (ETag) of current data at the defined database path.
   *
   * @return String of unique identifier.
   */
  template <typename T = const char *>
  String getETag(FirebaseData &fbdo, T path) { return RTDB.getETag(&fbdo, path); }

  /** Get the shallowed data at a defined node path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path that is being read the data.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Return the child data with its value or JSON object (its values will be truncated to true).
   * The data can be read from FirebaseData object.
   */
  template <typename T = const char *>
  bool getShallowData(FirebaseData &fbdo, T path) { return RTDB.getShallowData(&fbdo, path); }

  /** Enable the library to use only classic HTTP GET and POST methods.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param flag Boolean value to enable.
   *
   * @note This option used to escape the Firewall restriction (if the device is connected through
   * Firewall) that allows only HTTP GET and POST
   * HTTP PATCH request was sent as PATCH which not affected by this option.
   */
  void enableClassicRequest(FirebaseData &fbdo, bool flag) { RTDB.enableClassicRequest(&fbdo, flag); }

  /** Set the virtual child node ".priority" to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which to set the priority value.
   * @param priority The priority value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note This allows us to set priority to any node other than a priority that set through setJSON,
   * pushJSON, updateNode, and updateNodeSilent functions.
   * The returned priority value from server can read from function [FirebaseData object].priority().
   */
  template <typename T = const char *>
  bool setPriority(FirebaseData &fbdo, T path, float priority) { return RTDB.setPriority(&fbdo, path, priority); }

  template <typename T = const char *>
  bool setPriorityAsync(FirebaseData &fbdo, T path, float &priority) { return RTDB.setPriorityAsync(&fbdo, path, priority); }

  /** Read the virtual child node ".priority" value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which to set the priority value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The priority value from server can read from function [FirebaseData object].priority().
   */
  template <typename T = const char *>
  bool getPriority(FirebaseData &fbdo, T path) { return RTDB.getPriority(&fbdo, path); }

  /** Append new integer value to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which integer value will be appended.
   * @param value The appended value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T1 = const char *, typename T2 = int>
  bool pushInt(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.pushInt(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2 = int>
  bool pushIntAsync(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.pushIntAsync(&fbdo, path, value); }

  /** Append new integer value and the virtual child ".priority" to the defined database path.
   */
  template <typename T1 = const char *, typename T2 = int>
  bool pushInt(FirebaseData &fbdo, T1 path, T2 value, float priority) { return RTDB.pushInt(&fbdo, path, value, priority); }

  template <typename T1 = const char *, typename T2 = int>
  bool pushIntAsync(FirebaseData &fbdo, T1 path, T2 value, float priority)
  {
    return RTDB.pushIntAsync(&fbdo, path, value, priority);
  }

  /** Append new float value to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which float value will be appended.
   * @param value The appended value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T = const char *>
  bool pushFloat(FirebaseData &fbdo, T path, float value) { return RTDB.pushFloat(&fbdo, path, value); }

  template <typename T = const char *>
  bool pushFloatAsync(FirebaseData &fbdo, T path, float value) { return RTDB.pushFloatAsync(&fbdo, path, value); }

  /** Append new float value and the virtual child ".priority" to the defined database path.
   */
  template <typename T = const char *>
  bool pushFloat(FirebaseData &fbdo, T path, float value, float priority)
  {
    return RTDB.pushFloat(&fbdo, path, value, priority);
  }

  template <typename T = const char *>
  bool pushFloatAsync(FirebaseData &fbdo, T path, float value, float priority)
  {
    return RTDB.pushFloat(&fbdo, path, value, priority);
  }

  /** Append new double value (8 bytes) to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which float value will be appended.
   * @param value The appended value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T = const char *>
  bool pushDouble(FirebaseData &fbdo, T path, double value) { return RTDB.pushDouble(&fbdo, path, value); }

  template <typename T = const char *>
  bool pushDoubleAsync(FirebaseData &fbdo, T path, double value) { return RTDB.pushDoubleAsync(&fbdo, path, value); }

  /** Append new double value (8 bytes) and the virtual child ".priority" to the defined database path.
   */
  template <typename T = const char *>
  bool pushDouble(FirebaseData &fbdo, T path, double value, float priority)
  {
    return RTDB.pushDouble(&fbdo, path, value, priority);
  }

  template <typename T = const char *>
  bool pushDoubleAsync(FirebaseData &fbdo, T path, double value, float priority)
  {
    return RTDB.pushDoubleAsync(&fbdo, path, value, priority);
  }

  /** Append new Boolean value to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which Boolean value will be appended.
   * @param value The appended value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().

  */
  template <typename T = const char *>
  bool pushBool(FirebaseData &fbdo, T path, bool value) { return RTDB.pushBool(&fbdo, path, value); }

  template <typename T = const char *>
  bool pushBoolAsync(FirebaseData &fbdo, T path, bool value) { return RTDB.pushBoolAsync(&fbdo, path, value); }

  /** Append the new Boolean value and the virtual child ".priority" to the defined database path.
   */
  template <typename T = const char *>
  bool pushBool(FirebaseData &fbdo, T path, bool value, float priority)
  {
    return RTDB.pushBool(&fbdo, path, value, priority);
  }

  template <typename T = const char *>
  bool pushBoolAsync(FirebaseData &fbdo, T path, bool value, float priority)
  {
    return RTDB.pushBoolAsync(&fbdo, path, value, priority);
  }

  /** Append a new string (text) to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which string will be appended.
   * @param value The appended value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key stored in Firebase Data object,
   * which can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool pushString(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.pushString(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushStringAsync(FirebaseData &fbdo, T1 path, const T2 value) { return RTDB.pushStringAsync(&fbdo, path, value); }

  /** Append new string (text) and the virtual child ".priority" to the defined database path.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool pushString(FirebaseData &fbdo, T1 path, T2 value, float priority)
  {
    return RTDB.pushString(&fbdo, path, value, priority);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushStringAsync(FirebaseData &fbdo, T1 path, T2 value, float priority)
  {
    return RTDB.pushStringAsync(&fbdo, path, value, priority);
  }

  /** Append new child nodes key and value (using FirebaseJson object) to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in FirebaseJson object will be appended.
   * @param json The appended FirebaseJson object.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T = const char *>
  bool pushJSON(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.pushJSON(&fbdo, path, &json); }

  template <typename T = const char *>
  bool pushJSONAsync(FirebaseData &fbdo, T path, FirebaseJson &json)
  {
    return RTDB.pushJSONAsync(&fbdo, path, &json);
  }

  /** Append new child node key and value (FirebaseJson object) and the virtual child ".priority" to the defined database path.
   */
  template <typename T = const char *>
  bool pushJSON(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.pushJSON(&fbdo, path, &json, priority);
  }

  template <typename T = const char *>
  bool pushJSONAsync(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.pushJSONAsync(&fbdo, path, &json, priority);
  }

  /** Append child node array (using FirebaseJsonArray object) to the defined database path.
   * This will replace any child nodes inside the defined path with array defined in FirebaseJsonArray object.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in FirebaseJsonArray object will be appended.
   * @param arr The appended FirebaseJsonArray object.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T = const char *>
  bool pushArray(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.pushArray(&fbdo, path, &arr); }

  template <typename T = const char *>
  bool pushArrayAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.pushArrayAsync(&fbdo, path, &arr); }

  /** Append FirebaseJsonArray object and virtual child ".priority" at the defined database path.
   */
  template <typename T = const char *>
  bool pushArray(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.pushArray(&fbdo, path, &arr, priority);
  }

  template <typename T = const char *>
  bool pushArrayAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.pushArrayAsync(&fbdo, path, &arr, priority);
  }

  /** Append new blob (binary data) to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which binary data will be appended.
   * @param blob Byte array of data.
   * @param size Size of the byte array.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T = const char *>
  bool pushBlob(FirebaseData &fbdo, T path, uint8_t *blob, size_t size) { return RTDB.pushBlob(&fbdo, path, blob, size); }

  template <typename T = const char *>
  bool pushBlobAsync(FirebaseData &fbdo, T path, uint8_t *blob, size_t size)
  {
    return RTDB.pushBlobAsync(&fbdo, path, blob, size);
  }

  /** Append new binary data from the file stores on SD card/Flash memory to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param storageType Type of storage to read file data, StorageType::FLASH or StorageType::SD.
   * @param path Target database path in which binary data from the file will be appended.
   * @param fileName File name included its path in SD card/Flash memory.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool pushFile(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.pushFile(&fbdo, getMemStorageType(storageType), path, fileName, callback);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushFileAsync(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.pushFileAsync(&fbdo, getMemStorageType(storageType), path, fileName, callback);
  }

  /** Append the new Firebase server's timestamp to the defined database path.*
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which timestamp will be appended.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note The new appended node's key will be stored in Firebase Data object,
   * which its value can be accessed via function [FirebaseData object].pushName().
   */
  template <typename T = const char *>
  bool pushTimestamp(FirebaseData &fbdo, T path) { return RTDB.pushTimestamp(&fbdo, path); }

  template <typename T = const char *>
  bool pushTimestampAsync(FirebaseData &fbdo, T path) { return RTDB.pushTimestampAsync(&fbdo, path); }

  /** Set integer data at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which integer data will be set.
   * @param value Integer value to set.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully stores in the database.
   * Call [FirebaseData object].intData will return the integer value of
   * payload returned from server.
   */
  template <typename T1 = const char *, typename T2 = int>
  bool setInt(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.setInt(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2 = int>
  bool setIntAsync(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.setIntAsync(&fbdo, path, value); }

  /** Set integer data and virtual child ".priority" at the defined database path.
   */
  template <typename T1 = const char *, typename T2 = int>
  bool setInt(FirebaseData &fbdo, T1 path, T2 value, float priority) { return RTDB.setInt(&fbdo, path, value, priority); }

  template <typename T1 = const char *, typename T2 = int>
  bool setIntAsync(FirebaseData &fbdo, T1 path, T2 value, float priority)
  {
    return RTDB.setIntAsync(&fbdo, path, value, priority);
  }

  /** Set integer data at the defined database path if defined database path's ETag matched the ETag value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which integer data will be set.
   * @param value Integer value to set.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].intData to get the current integer value.
   */
  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setInt(FirebaseData &fbdo, T1 path, T2 value, T3 ETag) { return RTDB.setInt(&fbdo, path, value, ETag); }

  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setIntAsync(FirebaseData &fbdo, T1 path, T2 value, T3 ETag) { return RTDB.setIntAsync(&fbdo, path, value, ETag); }

  /** Set integer data and the virtual child ".priority" if defined ETag matches at the defined database path
   */
  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setInt(FirebaseData &fbdo, T1 path, T2 value, float priority, T3 ETag)
  {
    return RTDB.setInt(&fbdo, path, value, priority, ETag);
  }

  template <typename T1 = const char *, typename T2 = int, typename T3 = const char *>
  bool setIntAsync(FirebaseData &fbdo, T1 path, T2 value, float priority, T3 ETag)
  {
    return RTDB.setIntAsync(&fbdo, path, value, priority, ETag);
  }

  /** Set float data at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which float data will be set.
   * @param value Float value to set.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * Call [FirebaseData object].floatData will return the float value of
   * payload returned from server.
   */
  template <typename T = const char *>
  bool setFloat(FirebaseData &fbdo, T path, float value) { return RTDB.setFloat(&fbdo, path, value); }

  template <typename T = const char *>
  bool setFloatAsync(FirebaseData &fbdo, T path, float value) { return RTDB.setFloatAsync(&fbdo, path, value); }

  /** Set float data and virtual child ".priority" at the defined database path.
   */
  template <typename T = const char *>
  bool setFloat(FirebaseData &fbdo, T path, float value, float priority)
  {
    return RTDB.setFloat(&fbdo, path, value, priority);
  }

  template <typename T = const char *>
  bool setFloatAsync(FirebaseData &fbdo, T path, float value, float priority)
  {
    return RTDB.setFloatAsync(&fbdo, path, value, priority);
  }

  /** Set float data at the defined database path if defined database path's ETag matched the ETag value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which float data will be set.
   * @param value Float value to set.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * Call [FirebaseData object].floatData will return the float value of payload returned from server.
   *
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].floatData to get the current float value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloat(FirebaseData &fbdo, T1 path, float value, T2 ETag) { return RTDB.setFloat(&fbdo, path, value, ETag); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloatAsync(FirebaseData &fbdo, T1 path, float value, T2 ETag)
  {
    return RTDB.setFloatAsync(&fbdo, path, value, ETag);
  }

  /** Set float data and the virtual child ".priority" if defined ETag matches at the defined database path
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloat(FirebaseData &fbdo, T1 path, float value, float priority, T2 ETag)
  {
    return RTDB.setFloat(&fbdo, path, value, priority, ETag);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setFloatAsync(FirebaseData &fbdo, T1 path, float value, float priority, T2 ETag)
  {
    return RTDB.setFloatAsync(&fbdo, path, value, priority, ETag);
  }

  /** Set double data at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which float data will be set.
   * @param value Double value to set.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * Call [FirebaseData object].doubleData will return the double value of the payload returned from the server.
   *
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place,
   * use printf("%.9lf\n", firebaseData.doubleData()); for print the returned double value up to 9 decimal places.
   */
  template <typename T = const char *>
  bool setDouble(FirebaseData &fbdo, T path, double value) { return RTDB.setDouble(&fbdo, path, value); }

  template <typename T = const char *>
  bool setDoubleAsync(FirebaseData &fbdo, T path, double value) { return RTDB.setDoubleAsync(&fbdo, path, value); }

  /** Set double data and virtual child ".priority" at the defined database path.
   */
  template <typename T = const char *>
  bool setDouble(FirebaseData &fbdo, T path, double value, float priority)
  {
    return RTDB.setDouble(&fbdo, path, value, priority);
  }

  template <typename T = const char *>
  bool setDoubleAsync(FirebaseData &fbdo, T path, double value, float priority)
  {
    return RTDB.setDoubleAsync(&fbdo, path, value, priority);
  }

  /** Set double data at the defined database path if defined database path's ETag matched the ETag value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which float data will be set.
   * @param value Double value to set.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * Call [FirebaseData object].doubleData will return the double value of the payload returned from the server.
   *
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].doubeData to get the current double value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setDouble(FirebaseData &fbdo, T1 path, double value, T2 ETag) { return RTDB.setDouble(&fbdo, path, value, ETag); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setDoubleAsync(FirebaseData &fbdo, T1 path, double value, T2 ETag)
  {
    return RTDB.setDoubleAsync(&fbdo, path, value, ETag);
  }

  /** Set double data and the virtual child ".priority" if defined ETag matches at the defined database path
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setDouble(FirebaseData &fbdo, T1 path, double value, float priority, T2 ETag)
  {
    return RTDB.setDouble(&fbdo, path, value, priority, ETag);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setDoubleAsync(FirebaseData &fbdo, T1 path, double value, float priority, T2 ETag)
  {
    return RTDB.setDoubleAsync(&fbdo, path, value, priority, ETag);
  }

  /** Set Boolean data at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which Boolean data will be set.
   * @param value Boolean value to set.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * Call [FirebaseData object].boolData will return the Boolean value of
   * the payload returned from the server.
   */
  template <typename T = const char *>
  bool setBool(FirebaseData &fbdo, T path, bool value) { return RTDB.setBool(&fbdo, path, value); }

  template <typename T = const char *>
  bool setBoolAsync(FirebaseData &fbdo, T path, bool value) { return RTDB.setBoolAsync(&fbdo, path, value); }

  /** Set boolean data and virtual child ".priority" at the defined database path.
   */
  template <typename T = const char *>
  bool setBool(FirebaseData &fbdo, T path, bool value, float priority)
  {
    return RTDB.setBool(&fbdo, path, value, priority);
  }

  template <typename T = const char *>
  bool setBoolAsync(FirebaseData &fbdo, T path, bool value, float priority)
  {
    return RTDB.setBoolAsync(&fbdo, path, value, priority);
  }

  /** Set Boolean data at the defined database path if defined database path's ETag matched the ETag value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which Boolean data will be set.
   * @param value Boolean value to set.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   * Call [FirebaseData object].boolData will return the Boolean value of
   * the payload returned from the server.
   *
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].doubeData to get the current boolean value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setBool(FirebaseData &fbdo, T1 path, bool value, T2 ETag) { return RTDB.setBool(&fbdo, path, value, ETag); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setBoolAsync(FirebaseData &fbdo, T1 path, bool value, T2 ETag)
  {
    return RTDB.setBoolAsync(&fbdo, path, value, ETag);
  }

  /** Set boolean data and the virtual child ".priority" if defined ETag matches at the defined database path
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setBool(FirebaseData &fbdo, T1 path, bool value, float priority, T2 ETag)
  {
    return RTDB.setBool(&fbdo, path, value, priority, ETag);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setBoolAsync(FirebaseData &fbdo, T1 path, bool value, float priority, T2 ETag)
  {
    return RTDB.setBoolAsync(&fbdo, path, value, priority, ETag);
  }

  /** Set string (text) at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which string data will be set.
   * @param value String or text to set.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully
   * stores in the database.
   *
   * Call [FirebaseData object].stringData will return the string value of
   * the payload returned from the server.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setString(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.setString(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setStringAsync(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.setStringAsync(&fbdo, path, value); }

  /** Set string data and virtual child ".priority" at the defined database path.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setString(FirebaseData &fbdo, T1 path, T2 value, float priority)
  {
    return RTDB.setString(&fbdo, path, value, priority);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setStringAsync(FirebaseData &fbdo, T1 path, T2 value, float priority)
  {
    return RTDB.setStringAsync(&fbdo, path, value, priority);
  }

  /** Set string (text) at the defined database path if defined database path's ETag matched the ETag value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path in which string data will be set.
   * @param value String or text to set.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].stringData will return the string value of
   * the payload returned from the server.
   *
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also, call [FirebaseData object].stringData to get the current string value.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setString(FirebaseData &fbdo, T1 path, T2 value, T3 ETag) { return RTDB.setString(&fbdo, path, value, ETag); }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setStringAsync(FirebaseData &fbdo, T1 path, T2 value, T3 ETag) { return RTDB.setStringAsync(&fbdo, path, value, ETag); }

  /** Set string data and the virtual child ".priority" if defined ETag matches at the defined database path
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setString(FirebaseData &fbdo, T1 path, T2 value, float priority, T3 ETag)
  {
    return RTDB.setString(&fbdo, path, value, priority, ETag);
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setStringAsync(FirebaseData &fbdo, T1 path, T2 value, float priority, T3 ETag)
  {
    return RTDB.setStringAsync(&fbdo, path, value, priority, ETag);
  }

  /** Set the child node key and value (using FirebaseJson object) to the defined database path.
   * This will replace any child nodes inside the defined path with node' s key
   * and value defined in FirebaseJson object.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in FirebaseJson object will be replaced or set.
   * @param json The FirebaseJson object.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].jsonData will return the JSON string value of payload returned from server.
   */
  template <typename T = const char *>
  bool setJSON(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.setJSON(&fbdo, path, &json); }

  template <typename T = const char *>
  bool setJSONAsync(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.setJSONAsync(&fbdo, path, &json); }

  /** Set JSON data or FirebaseJson object and virtual child ".priority" at the defined database path.
   */
  template <typename T = const char *>
  bool setJSON(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.setJSON(&fbdo, path, &json, priority);
  }

  template <typename T = const char *>
  bool setJSONAsync(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.setJSONAsync(&fbdo, path, &json, priority);
  }

  /** Set child node key and value (using JSON data or FirebaseJson object) to the defined database path
   * if defined database path's ETag matched the ETag value.
   * This will replace any child nodes inside the defined path with node' s key
   * and value defined in JSON data or FirebaseJson object.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in JSON data will be replaced or set.
   * @param json The FirebaseJson object.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].jsonData will return the JSON string value of
   * payload returned from server.
   *
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].jsonData to get the current JSON string value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSON(FirebaseData &fbdo, T1 path, FirebaseJson &json, T2 ETag) { return RTDB.setJSON(&fbdo, path, &json, ETag); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSONAsync(FirebaseData &fbdo, T1 path, FirebaseJson &json, T2 ETag)
  {
    return RTDB.setJSONAsync(&fbdo, path, &json, ETag);
  }

  /** Set JSON data or FirebaseJson object and the virtual child ".priority" if defined ETag matches at the defined database path
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSON(FirebaseData &fbdo, T1 path, FirebaseJson &json, float priority, T2 ETag)
  {
    return RTDB.setJSON(&fbdo, path, &json, priority, ETag);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setJSONAsync(FirebaseData &fbdo, T1 path, FirebaseJson &json, float priority, T2 ETag)
  {
    return RTDB.setJSONAsync(&fbdo, path, &json, priority, ETag);
  }

  /** Set child node array (using FirebaseJsonArray object) to the defined database path.
   * This will replace any child nodes inside the defined path with array defined in FirebaseJsonArray object.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in FirebaseJsonArray object will be replaced or set.
   * @param arr The FirebaseJsonArray object.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully stores in the database.
   *
   * Call [FirebaseData object].jsonArray will return pointer to FirebaseJsonArray object contains array
   * payload returned from server, get the array payload using FirebaseJsonArray *arr = firebaseData.jsonArray();
   */
  template <typename T = const char *>
  bool setArray(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.setArray(&fbdo, path, &arr); }

  template <typename T = const char *>
  bool setArrayAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.setArrayAsync(&fbdo, path, &arr); }

  /** Set FirebaseJsonArray object and virtual child ".priority" at the defined database path.
   */
  template <typename T = const char *>
  bool setArray(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.setArray(&fbdo, path, &arr, priority);
  }

  template <typename T = const char *>
  bool setArrayAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.setArrayAsync(&fbdo, path, &arr, priority);
  }

  /** Set array (using JSON data or FirebaseJson object) to the defined database path if defined database path's ETag matched the ETag value.
   * This will replace any child nodes inside the defined path with array defined in FirebaseJsonArray object.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in JSON data will be replaced or set.
   * @param arr The FirebaseJsonArray object.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].jsonArray will return pointer to FirebaseJsonArray object contains array
   * payload returned from server, get the array payload using FirebaseJsonArray *arr = firebaseData.jsonArray();
   *
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   *
   * If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.
   * Also call [FirebaseData object].jsonArray to get the pointer to FirebaseJsonArray object of current array value.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setArray(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, T2 ETag) { return RTDB.setArray(&fbdo, path, &arr, ETag); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setArrayAsync(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, T2 ETag)
  {
    return RTDB.setArrayAsync(&fbdo, path, &arr, ETag);
  }

  /** Set FirebaseJsonArray object and the virtual child ".priority" if defined ETag matches at the defined database path
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setArray(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, float priority, T2 ETag)
  {
    return RTDB.setArray(&fbdo, path, &arr, priority, ETag);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setArrayAsync(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, float priority, T2 ETag)
  {
    return RTDB.setArrayAsync(&fbdo, path, &arr, priority, ETag);
  }

  /** Set blob (binary data) at the defined database path.
   * This will replace any child nodes inside the defined path with a blob or binary data.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which binary data will be set.
   * @param blob Byte array of data.
   * @param size Size of the byte array.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note No payload returned from the server.
   */
  template <typename T = const char *>
  bool setBlob(FirebaseData &fbdo, T path, uint8_t *blob, size_t size) { return RTDB.setBlob(&fbdo, path, blob, size); }

  template <typename T = const char *>
  bool setBlobAsync(FirebaseData &fbdo, T path, uint8_t *blob, size_t size)
  {
    return RTDB.setBlobAsync(&fbdo, path, blob, size);
  }

  /** Set blob (binary data) at the defined database path if defined database path's ETag matched the ETag value.
   * This will replace any child nodes inside the defined path with a blob or binary data.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which binary data will be set.
   * @param blob Byte array of data.
   * @param size Size of the byte array.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note No payload returned from the server.
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setBlob(FirebaseData &fbdo, T1 path, uint8_t *blob, size_t size, T2 ETag)
  {
    return RTDB.setBlob(&fbdo, path, blob, size, ETag);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setBlobAsync(FirebaseData &fbdo, T1 path, uint8_t *blob, size_t size, T2 ETag)
  {
    return RTDB.setBlobAsync(&fbdo, path, blob, size, ETag);
  }

  /** Set binary data from the file store on SD card/Flash memory to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param storageType Type of storage to read file data, StorageType::FLASH or StorageType::SD.
   * @param path Target database path in which binary data from the file will be set.
   * @param fileName File name included its path in SD card/Flash memory
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean type status indicates the success of the operation.
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   *
   * @note No payload returned from the server.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool setFile(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path, fileName, callback);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool setFileAsync(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path, fileName, callback);
  }

  /** Set binary data from file stored on SD card/Flash memory to the defined database path if defined database path's ETag matched the ETag value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param storageType Type of storage to read file data, StorageType::FLASH or StorageType::SD.
   * @param path Target database path in which binary data from the file will be set.
   * @param fileName File name included its path in SD card/Flash memory.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note No payload returned from the server.
   *
   * If ETag at the defined database path does not match the provided ETag parameter,
   * the operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setFile(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, T3 ETag, RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path, fileName, ETag, callback);
  }

  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  bool setFileAsync(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, T3 ETag,
                    RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path, fileName, ETag, callback);
  }

  /** Set Firebase server's timestamp to the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which timestamp will be set.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].intData will return the integer value of timestamp in seconds
   * or [FirebaseData object].doubleData to get millisecond timestamp.
   *
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place,
   * use printf("%.0lf\n", firebaseData.doubleData());.
   */
  template <typename T = const char *>
  bool setTimestamp(FirebaseData &fbdo, T path) { return RTDB.setTimestamp(&fbdo, path); }

  template <typename T = const char *>
  bool setTimestampAsync(FirebaseData &fbdo, T path) { return RTDB.setTimestampAsync(&fbdo, path); }

  /** Update the child node key or existing key's value (using FirebaseJson object) under the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in FirebaseJson object will be updated.
   * @param json The FirebaseJson object used for the update.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].jsonData will return the json string value of payload returned from server.
   * To reduce network data usage, use updateNodeSilent instead.
   */
  template <typename T = const char *>
  bool updateNode(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.updateNode(&fbdo, path, &json); }

  template <typename T = const char *>
  bool updateNodeAsync(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.updateNodeAsync(&fbdo, path, &json); }

  /** Update child node key or existing key's value and virtual child ".priority" (using JSON data or FirebaseJson object) under the defined database path.
   */
  template <typename T = const char *>
  bool updateNode(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.updateNode(&fbdo, path, &json, priority);
  }

  template <typename T = const char *>
  bool updateNodeAsync(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.updateNodeAsync(&fbdo, path, &json, priority);
  }

  /** Update the child node key or existing key's value (using FirebaseJson object) under the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Target database path which key and value in FirebaseJson object will be updated.
   * @param json The FirebaseJson object used for the update.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Owing to the objective of this function to reduce network data usage,
   * no payload will be returned from the server.
   */
  template <typename T = const char *>
  bool updateNodeSilent(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.updateNodeSilent(&fbdo, path, &json); }

  template <typename T = const char *>
  bool updateNodeSilentAsync(FirebaseData &fbdo, T path, FirebaseJson &json)
  {
    return RTDB.updateNodeSilentAsync(&fbdo, path, &json);
  }

  /** Update child node key or existing key's value and virtual child ".priority" (using JSON data or FirebaseJson object)
   * under the defined database path.
   */
  template <typename T = const char *>
  bool updateNodeSilent(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.updateNodeSilent(&fbdo, path, &json, priority);
  }

  template <typename T = const char *>
  bool updateNodeSilentAsync(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.updateNodeSilentAsync(&fbdo, path, &json, priority);
  }

  /** Read any type of value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the float value is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].to<int>(), [FirebaseData object].to<float>(), [FirebaseData object].to<std::vector<uint8_t> *>(),
   * [FirebaseData object].to<bool>(), [FirebaseData object].to<String>(), [FirebaseData object].to<FirebaseJson *> (pointer),
   * [FirebaseData object].to<FirebaseJsonArray *> (pointer) corresponded to its type from [FirebaseData object].dataType().
   */
  template <typename T = const char *>
  bool get(FirebaseData &fbdo, T path) { return RTDB.get(&fbdo, path); }

  /** Read the integer value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the float value is being read.

    @return Boolean type status indicates the success of the operation.

    Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.

    Call [FirebaseData object].intData will return the integer value of
    payload returned from server.

    If the type of payload returned from server is not integer, float and double,
    the function [FirebaseData object].intData will return zero (0).

  */
  template <typename T = const char *>
  bool getInt(FirebaseData &fbdo, T path) { return RTDB.getInt(&fbdo, path); }

  /** Read the integer value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the float value is being read.
   * @param target The integer type variable to store value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an integer, float and double,
   * the target variable's value will be zero (0).
   */
  template <typename T1 = const char *, typename T2>
  bool getInt(FirebaseData &fbdo, T1 path, T2 target) { return RTDB.getInt(&fbdo, path, target); }

  /** Read the float value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the float value is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].floatData will return the float value of
   * payload returned from server.
   *
   * If the payload returned from server is not integer, float and double,
   * the function [FirebaseData object].floatData will return zero (0).
   */
  template <typename T = const char *>
  bool getFloat(FirebaseData &fbdo, T path) { return RTDB.getFloat(&fbdo, path); }

  /** Read the float value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the float value is being read.
   * @param target The float type variable to store value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an integer, float and double,
   * the target variable's value will be zero (0).
   */
  template <typename T1 = const char *, typename T2>
  bool getFloat(FirebaseData &fbdo, T1 path, T2 target) { return RTDB.getFloat(&fbdo, path, target); }

  /** Read the double value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the float value is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].doubleData will return the double value of
   * the payload returned from the server.
   *
   * If the payload returned from server is not integer, float and double,
   * the function [FirebaseData object].doubleData will return zero (0).
   *
   * Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place,
   * use printf("%.9lf\n", firebaseData.doubleData()); for print value up to 9 decimal places.
   */
  template <typename T = const char *>
  bool getDouble(FirebaseData &fbdo, T path) { return RTDB.getDouble(&fbdo, path); }

  /** Read the float value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the float value is being read.
   * @param target The double type variable to store value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an integer, float and double,
   * the target variable's value will be zero (0).
   */
  template <typename T1 = const char *, typename T2>
  bool getDouble(FirebaseData &fbdo, T1 path, T2 target) { return RTDB.getDouble(&fbdo, path, target); }

  /** Read the Boolean value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the Boolean value is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].boolData will return the Boolean value of
   * the payload returned from the server.
   *
   * If the type of payload returned from the server is not Boolean,
   * the function [FirebaseData object].boolData will return false.
   */
  template <typename T = const char *>
  bool getBool(FirebaseData &fbdo, T path) { return RTDB.getBool(&fbdo, path); }

  /** Read the Boolean value at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the Boolean value is being read.
   * @param target The boolean type variable to store value.
   *  @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not Boolean,
   * the target variable's value will be false.
   */
  template <typename T1 = const char *, typename T2>
  bool getBool(FirebaseData &fbdo, T1 path, T2 target) { return RTDB.getBool(&fbdo, path, target); }

  /** Read the string at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the string value is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully
   * stores in the database.
   *
   * Call [FirebaseData object].stringData will return the string value of
   * the payload returned from the server.
   *
   * If the type of payload returned from the server is not a string,
   * the function [FirebaseData object].stringData will return empty string (String object).
   */
  template <typename T = const char *>
  bool getString(FirebaseData &fbdo, T path) { return RTDB.getString(&fbdo, path); }

  /** Read the string at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the string value is being read.
   * @param target The String object to store value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not a string,
   * the target String object's value will be empty.
   */
  template <typename T1 = const char *, typename T2>
  bool getString(FirebaseData &fbdo, T1 path, T2 target) { return RTDB.getString(&fbdo, path, target); }

  /** Read the JSON string at the defined database path.
   * The returned payload JSON string represents the child nodes and their value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the JSON string value is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully stores in the database.
   *
   * Call [FirebaseData object].jsonObject will return the pointer to FirebaseJson object contains the value of
   * the payload returned from the server.
   *
   * If the type of payload returned from server is not json,
   * the function [FirebaseData object].jsonObject will contain empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData &fbdo, T path) { return RTDB.getJSON(&fbdo, path); }

  /** Read the JSON string at the defined database path.
   * The returned the pointer to FirebaseJson that contains JSON payload represents the child nodes and their value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the JSON string value is being read.
   * @param target The FirebaseJson object pointer to get JSON data.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not JSON,
   * the target FirebaseJson object will contain an empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData &fbdo, T path, FirebaseJson *target) { return RTDB.getJSON(&fbdo, path, target); }

  /** Read the JSON string at the defined database path.
   * The returned the pointer to FirebaseJson that contains JSON payload represents the child nodes and their value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the JSON string value is being read.
   * @param query QueryFilter class to set query parameters to filter data.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Available query parameters for filtering the data are the following.
   *
   * QueryFilter.orderBy       Required parameter to specify which data used for data filtering included child key, key,
   *                           and value.
   *                           Use "$key" for filtering data by keys of all nodes at the defined database path.
   *                           Use "$value" for filtering data by value of all nodes at the defined database path.
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
   * Call [FirebaseData object].jsonObject will return the pointer to FirebaseJson object contains the value of
   * the payload returned from the server.
   *
   * If the type of payload returned from server is not json,
   * the function [FirebaseData object].jsonObject will contain empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData &fbdo, T path, QueryFilter &query) { return RTDB.getJSON(&fbdo, path, &query); }

  /** Read the JSON string at the defined database path as above
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the JSON string value is being read.
   * @param target The FirebaseJson object pointer to get JSON data.
   * @return Boolean type status indicates the success of the operation.
   *
   * If the type of payload returned from the server is not JSON,
   * the target FirebaseJson object will contain an empty object.
   */
  template <typename T = const char *>
  bool getJSON(FirebaseData &fbdo, T path, QueryFilter &query, FirebaseJson *target)
  {
    return RTDB.getJSON(&fbdo, path, &query, target);
  }

  /** Read the array data at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the array is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data that successfully
   * stores in the database.
   *
   * Call [FirebaseData object].jsonArray will return the pointer to FirebaseJsonArray object contains array value of
   * payload returned from server.
   *
   * If the type of payload returned from the server is not an array,
   * the array element in [FirebaseData object].jsonArray will be empty.
   */
  template <typename T = const char *>
  bool getArray(FirebaseData &fbdo, T path) { return RTDB.getArray(&fbdo, path); }

  /** Read the array data at the defined database path, and assign data to the target.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the array is being read.
   * @param target The FirebaseJsonArray object pointer to get array value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an array,
   * the target FirebaseJsonArray object will contain an empty array.
   */
  template <typename T = const char *>
  bool getArray(FirebaseData &fbdo, T path, FirebaseJsonArray *target) { return RTDB.getArray(&fbdo, path, target); }

  /** Read the array data at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the array is being read.
   * @param query QueryFilter class to set query parameters to filter data.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Available query parameters for filtering the data are the following.
   *
   * QueryFilter.orderBy       Required parameter to specify which data used for data filtering included child key, key,
   *                           and value.
   *                           Use "$key" for filtering data by keys of all nodes at the defined database path.
   *                           Use "$value" for filtering data by value of all nodes at the defined database path.
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
  template <typename T = const char *>
  bool getArray(FirebaseData &fbdo, T path, QueryFilter &query) { return RTDB.getArray(&fbdo, path, &query); }

  /** Read the array data at the defined database path as above
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the array is being read.
   * @param target The FirebaseJsonArray object to get array value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not an array,
   * the target FirebaseJsonArray object will contain an empty array.
   */
  template <typename T = const char *>
  bool getArray(FirebaseData &fbdo, T path, QueryFilter &query, FirebaseJsonArray *target)
  {
    return RTDB.getArray(&fbdo, path, &query, target);
  }

  /** Read the blob (binary data) at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the binary data is being read.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [FirebaseData object].blobData will return the dynamic array of unsigned 8-bit data (i.e. std::vector<uint8_t>) of
   * payload returned from server.
   *
   * If the type of payload returned from the server is not a blob,
   * the function [FirebaseData object].blobData will return empty array.
   */
  template <typename T = const char *>
  bool getBlob(FirebaseData &fbdo, T path) { return RTDB.getBlob(&fbdo, path); }

  /** Read the blob (binary data) at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path in which the binary data is being read.
   * @param target Dynamic array of unsigned 8-bit data (i.e. std::vector<uint8_t>) to store value.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If the type of payload returned from the server is not a blob,
   * the target variable value will be an empty array.
   */
  template <typename T = const char *>
  bool getBlob(FirebaseData &fbdo, T path, MB_VECTOR<uint8_t> &target) { return RTDB.getBlob(&fbdo, path, &target); }

  /** Download file data in a database at the defined database path and save it to SD card/Flash memory.
   * The downloaded data will be decoded to binary and save to SD card/Flash memory, then please make sure that data at
   * the defined database path is the file type.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param storageType Type of storage to write file data, StorageType::FLASH or StorageType::SD.
   * @param nodePath Database path that file data will be downloaded.
   * @param fileName File name included its path in SD card/Flash memory to save in SD card/Flash memory.
   * @param callback Optional. The callback function that accept RTDB_DownloadStatusInfo data.
   * @return Boolean type status indicates the success of the operation.
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool getFile(FirebaseData &fbdo, uint8_t storageType, T1 nodePath, T2 fileName, RTDB_DownloadProgressCallback callback = NULL)
  {
    return RTDB.getFile(&fbdo, getMemStorageType(storageType), nodePath, fileName, callback);
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
  bool downloadOTA(FirebaseData &fbdo, T fwPath, RTDB_DownloadProgressCallback callback = NULL)
  {
    return RTDB.downloadOTA(&fbdo, fwPath, callback);
  }

  /** Delete all child nodes at the defined database path.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path to be deleted.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool deleteNode(FirebaseData &fbdo, T path) { return RTDB.deleteNode(&fbdo, path); }

  /** Delete all child nodes at the defined database path if defined database path's ETag matched the ETag value.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path to be deleted.
   * @param ETag Known unique identifier string (ETag) of defined database path.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note If ETag at the defined database path does not match the provided ETag parameter,
   * he operation will fail with HTTP code 412, Precondition Failed (ETag is not matched).
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool deleteNode(FirebaseData &fbdo, T1 path, T2 ETag) { return RTDB.deleteNode(&fbdo, path, ETag); }

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
  template <typename T1 = const char *, typename T2 = const char *>
  bool deleteNodesByTimestamp(FirebaseData &fbdo, T1 path, T2 timestampNode, size_t limit, unsigned long dataRetentionPeriod)
  {
    return RTDB.deleteNodesByTimestamp(&fbdo, path, timestampNode, limit, dataRetentionPeriod);
  }

  /** Start subscribe to the value changes at the defined path and its children.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param path Database path to subscribe.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool beginStream(FirebaseData &fbdo, T path) { return RTDB.beginStream(&fbdo, path); }

  /** Start subscribe to the value changes at the defined parent node path with multiple nodes paths parsing.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param parentPath Database parent node path to subscribe.
   * @param childPath (deprecated) The string array of child nodes paths for parsing.
   * @param size The (deprecated) size of string array of child nodes paths for parsing.
   * @return Boolean type status indicates the success of the operation.
   */
  template <typename T = const char *>
  bool beginMultiPathStream(FirebaseData &fbdo, T parentPath) { return RTDB.beginMultiPathStream(&fbdo, parentPath); }

  /** Deprecated */
  template <typename T1 = const char *, typename T2 = const char *>
  bool beginMultiPathStream(FirebaseData &fbdo, T1 parentPath, T2 childPath, size_t size)
  {
    return RTDB.beginMultiPathStream(&fbdo, parentPath);
  }

  /** Read the stream event data at the defined database path.
   * Once beginStream was called e.g. in setup(), the readStream function
   * should call inside the loop function.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Using the same Firebase Data object for stream read/monitoring associated
   * with getXXX, setXXX, pushXXX, updateNode and deleteNode will break or quit
   * the current stream connection.
   *
   * he stream will be resumed or reconnected automatically when calling readStream.
   */
  bool readStream(FirebaseData &fbdo) { return RTDB.readStream(&fbdo); }

  /** End the stream connection at a defined path.
   * It can be restart again by calling beginStream.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Boolean type status indicates the success of the operation.
   */
  bool endStream(FirebaseData &fbdo) { return RTDB.endStream(&fbdo); }

  /** Set the stream callback functions.
   * setStreamCallback should be called before Firebase.beginStream.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param dataAvailablecallback a Callback function that accepts streamData parameter.
   * @param timeoutCallback Callback function will be called when the stream connection was timed out (optional).
   * @param streamTaskStackSize - The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).
   *
   * @note dataAvailablecallback will be called When data in the defined path changed or the stream path changed or
   * stream connection was resumed from getXXX, setXXX, pushXXX, updateNode, deleteNode.
   *
   * The payload returned from the server will be one of these integer, float, string, JSON and blob types.
   *
   * Call [streamData object].dataType to determine what type of data successfully stores in the database.
   *
   * Call [streamData object].xxxData will return the appropriate data type of
   * the payload returned from the server.
   */
#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
  void setStreamCallback(FirebaseData &fbdo, FirebaseData::StreamEventCallback dataAvailablecallback,
                         FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize = 8192)
  {
    RTDB.setStreamCallback(&fbdo, dataAvailablecallback, timeoutCallback, streamTaskStackSize);
  }
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO) || defined(FB_ENABLE_EXTERNAL_CLIENT)
  void setStreamCallback(FirebaseData &fbdo, FirebaseData::StreamEventCallback dataAvailablecallback,
                         FirebaseData::StreamTimeoutCallback timeoutCallback = NULL)
  {
    RTDB.setStreamCallback(&fbdo, dataAvailablecallback, timeoutCallback);
  }
#endif

  /** Set the multiple paths stream callback functions.
   * setMultiPathStreamCallback should be called before Firebase.beginMultiPathStream.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param multiPathDataCallback a Callback function that accepts MultiPathStreamData parameter.
   * @param timeoutCallback a Callback function will be called when the stream connection was timed out (optional).
   * @param streamTaskStackSize - The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).
   *
   * @note multiPathDataCallback will be called When data in the defined path changed or the stream path changed or stream
   * connection was resumed from getXXX, setXXX, pushXXX, updateNode, deleteNode.
   *
   * The payload returned from the server will be one of these integer, float, string and JSON.
   *
   * Call [MultiPathStreamData object].get to get the child node value, type and data path.
   *
   * The properties [MultiPathStreamData object].value, [MultiPathStreamData object].dataPath, and
   * [MultiPathStreamData object].type will return the value, path of data, and type of data respectively.
   *
   * These properties will store the result from calling the function [MultiPathStreamData object].get.
   */
#if defined(ESP8266)
  void setMultiPathStreamCallback(FirebaseData &fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback,
                                  FirebaseData::StreamTimeoutCallback timeoutCallback = NULL)
  {
    RTDB.setMultiPathStreamCallback(&fbdo, multiPathDataCallback, timeoutCallback);
  }
#elif defined(ESP32)
  void setMultiPathStreamCallback(FirebaseData &fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback,
                                  FirebaseData::StreamTimeoutCallback timeoutCallback = NULL, size_t streamTaskStackSize = 8192)
  {
    RTDB.setMultiPathStreamCallback(&fbdo, multiPathDataCallback, timeoutCallback, streamTaskStackSize);
  }
#endif

  /** Remove stream callback functions.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   */
  void removeStreamCallback(FirebaseData &fbdo) { RTDB.removeStreamCallback(&fbdo); }

  /** Remove multiple paths stream callback functions.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   */
  void removeMultiPathStreamCallback(FirebaseData &fbdo) { RTDB.removeMultiPathStreamCallback(&fbdo); }

  /** Run stream manually.
   * To manually triggering the stream callback function, this should call repeatedly in loop().
   */
  void runStream()
  {
    RTDB.mStopStreamLoopTask();
    RTDB.mRunStream();
  }

  /** Backup (download) database at the defined database path to SD card/Flash memory.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param storageType Type of storage to save file, StorageType::FLASH or StorageType::SD.
   * @param nodePath Database path to be backuped.
   * @param fileName File name to save.
   * @param callback Optional. The callback function that accept RTDB_DownloadStatusInfo data.
   * @return Boolean type status indicates the success of the operation.
   *
   * @note Only 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension) can be saved to SD card/Flash memory.
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool backup(FirebaseData &fbdo, uint8_t storageType, T1 nodePath, T2 fileName,
              RTDB_DownloadProgressCallback callback = NULL)
  {
    return RTDB.backup(&fbdo, getMemStorageType(storageType), nodePath, fileName, callback);
  }

  /** Restore database at a defined path using backup file saved on SD card/Flash memory.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param storageType Type of storage to read file, StorageType::FLASH or StorageType::SD.
   * @param nodePath Database path to  be restored.
   * @param fileName File name to read.
   * @param callback Optional. The callback function that accept RTDB_UploadStatusInfo data.
   * @return Boolean type status indicates the success of the operation.
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  template <typename T1 = const char *, typename T2 = const char *>
  bool restore(FirebaseData &fbdo, uint8_t storageType, T1 nodePath, T2 fileName, RTDB_UploadProgressCallback callback = NULL)
  {
    return RTDB.restore(&fbdo, getMemStorageType(storageType), nodePath, fileName, callback);
  }

  /** Set maximum Firebase read/store retry operation (0 255) in case of network problems and buffer overflow.
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param num The maximum retry.
   */
  void setMaxRetry(FirebaseData &fbdo, uint8_t num) { RTDB.setMaxRetry(&fbdo, num); }

#if defined(ENABLE_ERROR_QUEUE)
  /** Set the maximum Firebase Error Queues in the collection (0 255).
   * Firebase read/store operation causes by network problems and buffer overflow will be added to Firebase
   * Error Queues collection.
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param num The maximum Firebase Error Queues.
   */
  void setMaxErrorQueue(FirebaseData &fbdo, uint8_t num) { RTDB.setMaxErrorQueue(&fbdo, num); }

  /** Save Firebase Error Queues as SPIFFS file (save only database store queues).
   * Firebase read (get) operation will not be saved.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param filename Filename to be saved.
   * @param storageType Type of storage to save file, StorageType::FLASH or StorageType::SD.
   */
  template <typename T = const char *>
  bool saveErrorQueue(FirebaseData &fbdo, T filename, uint8_t storageType)
  {
    return RTDB.saveErrorQueue(&fbdo, filename, getMemStorageType(storageType));
  }

  /** Delete file in Flash (SPIFFS) or SD card.
   *
   * @param filename File name to delete.
   * @param storageType Type of storage to save file, StorageType::FLASH or StorageType::SD.
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  bool deleteStorageFile(const String &filename, uint8_t storageType)
  {
    return RTDB.deleteStorageFile(filename.c_str(), getMemStorageType(storageType));
  }

  /** estore Firebase Error Queues from the SPIFFS file.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param filename Filename to be read and restore queues.
   * @param storageType Type of storage to read file, StorageType::FLASH or StorageType::SD.
   */
  template <typename T = const char *>
  bool restoreErrorQueue(FirebaseData &fbdo, T filename, uint8_t storageType)
  {
    return RTDB.restoreErrorQueue(&fbdo, filename, getMemStorageType(storageType));
  }

  /** Determine the number of Firebase Error Queues stored in a defined SPIFFS file.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param filename Filename to be read and count for queues.
   * @param storageType Type of storage to read file, StorageType::FLASH or StorageType::SD.
   * @return Number (0-255) of queues store in defined SPIFFS file.
   *
   * The file systems for flash and sd memory can be changed in FirebaseFS.h.
   */
  template <typename T = const char *>
  uint8_t errorQueueCount(FirebaseData &fbdo, T filename, uint8_t storageType)
  {
    return RTDB.errorQueueCount(&fbdo, filename, getMemStorageType(storageType));
  }

  /** Determine number of queues in Firebase Data object Firebase Error Queues collection.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Number (0-255) of queues in Firebase Data object queue collection.
   */
  uint8_t errorQueueCount(FirebaseData &fbdo) { return RTDB.errorQueueCount(&fbdo); }

  /** Determine whether the  Firebase Error Queues collection was full or not.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Boolean type status indicates whether the  Firebase Error Queues collection was full or not.
   */
  bool isErrorQueueFull(FirebaseData &fbdo) { return RTDB.isErrorQueueFull(&fbdo); }

  /** Process all failed Firebase operation queue items when the network is available.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param callback a Callback function that accepts QueueInfo parameter.
   */
  void processErrorQueue(FirebaseData &fbdo, FirebaseData::QueueInfoCallback callback = NULL)
  {
    return RTDB.processErrorQueue(&fbdo, callback);
  }

  /** Return Firebase Error Queue ID of last Firebase Error.
   * Return 0 if there is no Firebase Error from the last operation.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Number of Queue ID.
   */
  uint32_t getErrorQueueID(FirebaseData &fbdo) { return RTDB.getErrorQueueID(&fbdo); }

  /** Determine whether the Firebase Error Queue currently exists is Error Queue collection or not.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param errorQueueID The Firebase Error Queue ID get from getErrorQueueID.
   * @return Boolean type status indicates the queue existence.
   */
  bool isErrorQueueExisted(FirebaseData &fbdo, uint32_t errorQueueID) { return RTDB.isErrorQueueExisted(&fbdo, errorQueueID); }

  /** Start the Firebase Error Queues Auto Run Process.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @param callback a Callback function that accepts QueueInfo Object as a parameter, optional.
   *
   * @note The following functions are available from QueueInfo Object accepted by callback.
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
  void beginAutoRunErrorQueue(FirebaseData &fbdo, FirebaseData::QueueInfoCallback callback = NULL)
  {
    RTDB.beginAutoRunErrorQueue(&fbdo, callback);
  }

  /** Stop the Firebase Error Queues Auto Run Process.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   */
  void endAutoRunErrorQueue(FirebaseData &fbdo) { RTDB.endAutoRunErrorQueue(&fbdo); }

  /** Clear all Firbase Error Queues in Error Queue collection.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   */
  void clearErrorQueue(FirebaseData &fbdo) { RTDB.clearErrorQueue(&fbdo); };

#endif

#endif

    /** Send Firebase Cloud Messaging to the device with the first registration token which added by
     *  firebaseData.fcm.addDeviceToken.
     *
     * @param fbdo Firebase Data Object to hold data and instance.
     * @param index The index (starts from 0) of recipient device token which added by firebaseData.fcm.addDeviceToken
     * @return Boolean type status indicates the success of the operation.
     */
#ifdef ENABLE_FCM
  bool sendMessage(FirebaseData &fbdo, uint16_t index);
#endif

  /** Send Firebase Cloud Messaging to all devices (multicast) which added by firebaseData.fcm.addDeviceToken.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Boolean type status indicates the success of the operation.
   */
#ifdef ENABLE_FCM
  bool broadcastMessage(FirebaseData &fbdo);
#endif

  /** Send Firebase Cloud Messaging to devices that subscribed to the topic.
   *
   * @param fbdo Firebase Data Object to hold data and instance.
   * @return Boolean type status indicates the success of the operation.
   */
#ifdef ENABLE_FCM
  bool sendTopic(FirebaseData &fbdo);
#endif

#if defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)

  /** Initiate SD card with SPI port configuration.
   *
   * @param ss SPI Chip/Slave Select pin.
   * @param sck SPI Clock pin.
   * @param miso SPI MISO pin.
   * @param mosi SPI MOSI pin.
   * @param frequency The SPI frequency
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, uint32_t frequency = 4000000);

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)

  /** Initiate SD card with SD FS configurations (ESP8266 only).
   *
   * @param ss SPI Chip/Slave Select pin.
   * @param sdFSConfig The pointer to SDFSConfig object (ESP8266 only).
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(SDFSConfig *sdFSConfig);

#endif

#if defined(ESP32)
  /** Initiate SD card with chip select and SPI configuration (ESP32 only).
   *
   * @param ss SPI Chip/Slave Select pin.
   * @param spiConfig The pointer to SPIClass object for SPI configuartion.
   * @param frequency The SPI frequency.
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(int8_t ss, SPIClass *spiConfig = nullptr, uint32_t frequency = 4000000);
#endif

#if defined(MBFS_ESP32_SDFAT_ENABLED) || defined(MBFS_SDFAT_ENABLED)
  /** Initiate SD card with SdFat SPI and pins configurations (with SdFat included only).
   *
   * @param sdFatSPIConfig The pointer to SdSpiConfig object for SdFat SPI configuration.
   * @param ss SPI Chip/Slave Select pin.
   * @param sck SPI Clock pin.
   * @param miso SPI MISO pin.
   * @param mosi SPI MOSI pin.
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);

  /** Initiate SD card with SdFat SDIO configuration (with SdFat included only).
   *
   * @param sdFatSDIOConfig The pointer to SdioConfig object for SdFat SDIO configuration.
   * @return Boolean type status indicates the success of the operation.
   */
  bool sdBegin(SdioConfig *sdFatSDIOConfig);

#endif

#endif

#if defined(ESP32) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD_MMC)
  /** Initialize the SD_MMC card (ESP32 only).
   *
   * @param mountpoint The mounting point.
   * @param mode1bit Allow 1 bit data line (SPI mode).
   * @param format_if_mount_failed Format SD_MMC card if mount failed.
   * @return The boolean value indicates the success of operation.
   */
  bool sdMMCBegin(const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false);
#endif

  /** Set system time with timestamp.
   *
   * @param ts timestamp in seconds from midnight Jan 1, 1970.
   * @return Boolean type status indicates the success of the operation.
   */
  bool setSystemTime(time_t ts);

  /** Provide the http code error string
   *
   * @param httpCode The http code.
   * @param buff The String buffer out.
   */
  void errorToString(int httpCode, String &buff)
  {
    MB_String out;
    Signer.errorToString(httpCode, out);
    buff = out.c_str();
  }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.set(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.setAsync(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, T1 path, T2 value, float priority) { return RTDB.set(&fbdo, path, value, priority); }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, T1 path, T2 value, float priority) { return RTDB.setAsync(&fbdo, path, value, priority); }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool set(FirebaseData &fbdo, T1 path, T2 value, T3 ETag) { return RTDB.set(&fbdo, path, value, ETag); }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool setAsync(FirebaseData &fbdo, T1 path, T2 value, T3 ETag) { return RTDB.setAsync(&fbdo, path, value, ETag); }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool set(FirebaseData &fbdo, T1 path, T2 value, float priority, T3 ETag) { return RTDB.set(&fbdo, path, value, priority, ETag); }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool setAsync(FirebaseData &fbdo, T1 path, T2 value, float priority, T3 ETag)
  {
    return RTDB.setAsync(&fbdo, path, value, priority, ETag);
  }

  template <typename T = const char *>
  bool set(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.setJSON(&fbdo, path, &json); }

  template <typename T = const char *>
  bool setAsync(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.setJSONAsync(&fbdo, path, &json); }

  template <typename T = const char *>
  bool set(FirebaseData &fbdo, T path, FirebaseJson &json, float priority) { return RTDB.setJSON(&fbdo, path, &json, priority); }

  template <typename T = const char *>
  bool setAsync(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.setJSONAsync(&fbdo, path, &json, priority);
  }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, T1 path, FirebaseJson &json, T2 ETag) { return RTDB.setJSON(&fbdo, path, &json, ETag); }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, T1 path, FirebaseJson &json, T2 ETag) { return RTDB.setJSONAsync(&fbdo, path, &json, ETag); }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, T1 path, FirebaseJson &json, float priority, T2 ETag)
  {
    return RTDB.setJSON(&fbdo, path, &json, priority, ETag);
  }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, T1 path, FirebaseJson &json, float priority, T2 ETag)
  {
    return RTDB.setJSONAsync(&fbdo, path, &json, priority, ETag);
  }

  template <typename T = const char *>
  bool set(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.setArray(&fbdo, path, &arr); }

  template <typename T = const char *>
  bool setAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.setArrayAsync(&fbdo, path, &arr); }

  template <typename T = const char *>
  bool set(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.setArray(&fbdo, path, &arr, priority);
  }

  template <typename T = const char *>
  bool setAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.setArrayAsync(&fbdo, path, &arr, priority);
  }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, T2 ETag) { return RTDB.setArray(&fbdo, path, &arr, ETag); }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, T2 ETag)
  {
    return RTDB.setArrayAsync(&fbdo, path, &arr, ETag);
  }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, float priority, T2 ETag)
  {
    return RTDB.setArray(&fbdo, path, &arr, priority, ETag);
  }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, T1 path, FirebaseJsonArray &arr, float priority, T2 ETag)
  {
    return RTDB.setArrayAsync(&fbdo, path, &arr, priority, ETag);
  }

  template <typename T = const char *>
  bool set(FirebaseData &fbdo, T path, uint8_t *blob, size_t size) { return RTDB.setBlob(&fbdo, path, blob, size); }

  template <typename T = const char *>
  bool setAsync(FirebaseData &fbdo, T path, uint8_t *blob, size_t size) { return RTDB.setBlobAsync(&fbdo, path, blob, size); }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, T1 path, uint8_t *blob, size_t size, T2 ETag)
  {
    return RTDB.setBlob(&fbdo, path, blob, size, ETag);
  }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, T1 path, uint8_t *blob, size_t size, T2 ETag)
  {
    return RTDB.setBlobAsync(&fbdo, path, blob, size, ETag);
  }

  template <typename T1 = const char *, typename T2>
  bool set(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName)
  {
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path, fileName);
  }

  template <typename T1 = const char *, typename T2>
  bool setAsync(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName)
  {
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path, fileName);
  }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool set(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, T3 ETag)
  {
    return RTDB.setFile(&fbdo, getMemStorageType(storageType), path, fileName, ETag);
  }

  template <typename T1 = const char *, typename T2, typename T3 = const char *>
  bool setAsync(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName, T3 ETag)
  {
    return RTDB.setFileAsync(&fbdo, getMemStorageType(storageType), path, fileName, ETag);
  }

  template <typename T1 = const char *, typename T2>
  bool push(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.push(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2>
  bool pushAsync(FirebaseData &fbdo, T1 path, T2 value) { return RTDB.pushAsync(&fbdo, path, value); }

  template <typename T1 = const char *, typename T2>
  bool push(FirebaseData &fbdo, T1 path, T2 value, float priority) { return RTDB.push(&fbdo, path, value, priority); }

  template <typename T1 = const char *, typename T2>
  bool pushAsync(FirebaseData &fbdo, T1 path, T2 value, float priority) { return RTDB.pushAsync(&fbdo, path, value, priority); }

  template <typename T = const char *>
  bool push(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.pushJSON(&fbdo, path, &json); }

  template <typename T = const char *>
  bool pushAsync(FirebaseData &fbdo, T path, FirebaseJson &json) { return RTDB.pushJSONAsync(&fbdo, path, &json); }

  template <typename T = const char *>
  bool push(FirebaseData &fbdo, T path, FirebaseJson &json, float priority) { return RTDB.pushJSON(&fbdo, path, &json, priority); }

  template <typename T = const char *>
  bool pushAsync(FirebaseData &fbdo, T path, FirebaseJson &json, float priority)
  {
    return RTDB.pushJSONAsync(&fbdo, path, &json, priority);
  }

  template <typename T = const char *>
  bool push(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.pushArray(&fbdo, path, &arr); }

  template <typename T = const char *>
  bool pushAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr) { return RTDB.pushArrayAsync(&fbdo, path, &arr); }

  template <typename T = const char *>
  bool push(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.pushArray(&fbdo, path, &arr, priority);
  }

  template <typename T = const char *>
  bool pushAsync(FirebaseData &fbdo, T path, FirebaseJsonArray &arr, float priority)
  {
    return RTDB.pushArrayAsync(&fbdo, path, &arr, priority);
  }

  template <typename T = const char *>
  bool push(FirebaseData &fbdo, T path, uint8_t *blob, size_t size) { return RTDB.pushBlob(&fbdo, path, blob, size); }

  template <typename T = const char *>
  bool pushAsync(FirebaseData &fbdo, T path, uint8_t *blob, size_t size) { return RTDB.pushBlobAsync(&fbdo, path, blob, size); }

  template <typename T1 = const char *, typename T2 = const char *>
  bool push(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName)
  {
    return RTDB.pushFile(&fbdo, getMemStorageType(storageType), path, fileName);
  }

  template <typename T1 = const char *, typename T2 = const char *>
  bool pushAsync(FirebaseData &fbdo, uint8_t storageType, T1 path, T2 fileName)
  {
    return RTDB.pushFileAsync(&fbdo, getMemStorageType(storageType), path, fileName);
  }

private:
#ifdef ENABLE_FCM
  bool handleFCMRequest(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType);
#endif
  fb_esp_mem_storage_type getMemStorageType(uint8_t old_type);
  void init(FirebaseConfig *config, FirebaseAuth *auth);
  bool mSignUp(FirebaseConfig *config, FirebaseAuth *auth, MB_StringPtr email, MB_StringPtr password);
  void mSetAuthToken(FirebaseConfig *config, MB_StringPtr authToken, size_t expire, MB_StringPtr refreshToken,
                     fb_esp_auth_token_type type, MB_StringPtr clientId, MB_StringPtr clientSecret);
  bool msendEmailVerification(FirebaseConfig *config, MB_StringPtr idToken);
  bool mDeleteUser(FirebaseConfig *config, FirebaseAuth *auth, MB_StringPtr idToken);
  bool mSendResetPassword(FirebaseConfig *config, MB_StringPtr email);

  void pre_begin(MB_StringPtr databaseURL, MB_StringPtr databaseSecret)
  {
    if (!config)
      config = new FirebaseConfig();

    if (!auth)
      auth = new FirebaseAuth();

    extConfig = false;

    config->database_url = databaseURL;
    config->signer.tokens.legacy_token = addrTo<const char *>(databaseSecret.address());
  }

  FirebaseAuth *auth = nullptr;
  FirebaseConfig *config = nullptr;
  MB_FS mbfs;
  uint32_t mb_ts = 0;
  uint32_t mb_ts_offset = 0;
  bool extConfig = true;
};

extern FIREBASE_CLASS Firebase;

#endif /* FIREBASE_ESP32_CLIENT || FIREBASE_ESP8266_CLIENT */

#endif /* ESP8266 || ESP32 */

#endif /* Firebase_H */