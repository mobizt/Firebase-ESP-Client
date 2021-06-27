/**
 * Google's Firebase ESP Client Main class, Firebase_ESP_Client.cpp v2.3.5
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created June 27, 2021
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 *
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person returning a copy
 * of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of
 * the Software, and to permit persons to whom the Software is furnished to do
 * so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FIREBASE_ESP_CLIENT_CPP
#define FIREBASE_ESP_CLIENT_CPP

#include "Firebase_ESP_Client.h"

Firebase_ESP_Client::Firebase_ESP_Client() {}

Firebase_ESP_Client::~Firebase_ESP_Client()
{
  if (ut)
    delete ut;

  if (auth)
    delete auth;

  if (cfg)
    delete cfg;
}

void Firebase_ESP_Client::begin(FirebaseConfig *config, FirebaseAuth *auth)
{
  init(config, auth);

  if (cfg->service_account.json.path.length() > 0)
  {
    if (!Signer.parseSAFile())
      cfg->signer.tokens.status = token_status_uninitialized;
  }

  if (cfg->signer.tokens.legacy_token.length() > 0)
    Signer.setTokenType(token_type_legacy_token);
  else if (Signer.tokenSigninDataReady())
  {
    if (auth->token.uid.length() == 0)
      Signer.setTokenType(token_type_oauth2_access_token);
    else
      Signer.setTokenType(token_type_custom_token);
  }
  else if (Signer.userSigninDataReady())
    Signer.setTokenType(token_type_id_token);

  struct fb_esp_url_info_t uinfo;
  cfg->_int.fb_auth_uri =
      cfg->signer.tokens.token_type == token_type_legacy_token ||
      cfg->signer.tokens.token_type == token_type_id_token;

  if (cfg->host.length() > 0)
    cfg->database_url = cfg->host;

  if (cfg->database_url.length() > 0)
  {
    ut->getUrlInfo(cfg->database_url.c_str(), uinfo);
    cfg->database_url = uinfo.host;
  }

  if (strlen_P(cfg->cert.data))
    cfg->_int.fb_caCert = cfg->cert.data;

  if (cfg->cert.file.length() > 0)
  {
    if (cfg->cert.file_storage == mem_storage_type_sd && !cfg->_int.fb_sd_rdy)
      cfg->_int.fb_sd_rdy = ut->sdTest(cfg->_int.fb_file);
    else if ((cfg->cert.file_storage == mem_storage_type_flash) &&
             !cfg->_int.fb_flash_rdy)
      ut->flashTest();
  }

  Signer.handleToken();
}

struct token_info_t Firebase_ESP_Client::authTokenInfo()
{
  return Signer.tokenInfo;
}

bool Firebase_ESP_Client::ready()
{
  return Signer.tokenReady();
}

bool Firebase_ESP_Client::authenticated()
{
  return Signer.authenticated;
}

void Firebase_ESP_Client::init(FirebaseConfig *config, FirebaseAuth *auth)
{
  this->auth = auth;
  cfg = config;

  if (!cfg)
    cfg = new FirebaseConfig();

  if (!this->auth)
    this->auth = new FirebaseAuth();

  if (ut)
    delete ut;

  ut = new UtilsClass(config);

#ifdef ENABLE_RTDB
  RTDB.begin(ut);
#endif
#ifdef ENABLE_FCM
  FCM.begin(ut);
#endif
#ifdef ENABLE_FB_STORAGE
  Storage.begin(ut);
#endif
#ifdef ENABLE_FIRESTORE
  Firestore.begin(ut);
#endif
#ifdef ENABLE_FB_FUNCTIONS
  Functions.begin(ut);
#endif
#ifdef ENABLE_GC_STORAGE
  GCStorage.begin(ut);
#endif

  cfg->_int.fb_reconnect_wifi = WiFi.getAutoReconnect();

  cfg->signer.signup = false;
  Signer.begin(ut, cfg, auth);
  ut->clearS(cfg->signer.tokens.error.message);
}

bool Firebase_ESP_Client::signUp(FirebaseConfig *config, FirebaseAuth *auth, const char *email, const char *password)
{
  init(config, auth);

  return Signer.getIdToken(true, email, password);
}

bool Firebase_ESP_Client::sendEmailVerification(FirebaseConfig *config, const char *idToken)
{
  init(config, nullptr);
  return Signer.handleEmailSending(idToken, fb_esp_user_email_sending_type_verify);
}

bool Firebase_ESP_Client::sendResetPassword(FirebaseConfig *config, const char *email)
{
  init(config, nullptr);
  return Signer.handleEmailSending(email, fb_esp_user_email_sending_type_reset_psw);
}

void Firebase_ESP_Client::reconnectWiFi(bool reconnect)
{
  WiFi.setAutoReconnect(reconnect);
}

void Firebase_ESP_Client::setFloatDigits(uint8_t digits)
{
  if (digits < 7)
    cfg->_int.fb_float_digits = digits;
}

void Firebase_ESP_Client::setDoubleDigits(uint8_t digits)
{
  if (digits < 9)
    cfg->_int.fb_double_digits = digits;
}

bool Firebase_ESP_Client::sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
  if (Signer.getCfg())
  {
    Signer.getCfg()->_int.sd_config.sck = sck;
    Signer.getCfg()->_int.sd_config.miso = miso;
    Signer.getCfg()->_int.sd_config.mosi = mosi;
    Signer.getCfg()->_int.sd_config.ss = ss;
  }
#if defined(ESP32)
  if (ss > -1)
  {
    SPI.begin(sck, miso, mosi, ss);
#if defined(CARD_TYPE_SD)
    return SD_FS.begin(ss, SPI);
#endif
    return false;
  }
  else
    return SD_FS.begin();
#elif defined(ESP8266)
  if (ss > -1)
    return SD_FS.begin(ss);
  else
    return SD_FS.begin(SD_CS_PIN);
#endif
  return false;
}

bool Firebase_ESP_Client::sdMMCBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
{
#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)
  if (Signer.getCfg())
  {
    Signer.getCfg()->_int.sd_config.sd_mmc_mountpoint = mountpoint;
    Signer.getCfg()->_int.sd_config.sd_mmc_mode1bit = mode1bit;
    Signer.getCfg()->_int.sd_config.sd_mmc_format_if_mount_failed = format_if_mount_failed;
  }
  return SD_FS.begin(mountpoint, mode1bit, format_if_mount_failed);
#endif
#endif
  return false;
}

bool Firebase_ESP_Client::setSystemTime(time_t ts)
{
  return ut->setTimestamp(ts) == 0;
}

Firebase_ESP_Client Firebase = Firebase_ESP_Client();

#endif