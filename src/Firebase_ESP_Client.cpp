/**
 * Google's Firebase ESP Client Main class, Firebase_ESP_Client.h version 2.2.0
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created May 17, 2021
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
}

void Firebase_ESP_Client::begin(FirebaseConfig *config, FirebaseAuth *auth)
{
  init(config, auth);

  if (_cfg->service_account.json.path.length() > 0)
  {
    if (!Signer.parseSAFile())
      _cfg->signer.tokens.status = token_status_uninitialized;
  }

  if (_cfg->signer.tokens.legacy_token.length() > 0)
    Signer.setTokenType(token_type_legacy_token);
  else if (Signer.tokenSigninDataReady())
  {
    if (_auth->token.uid.length() == 0)
      Signer.setTokenType(token_type_oauth2_access_token);
    else
      Signer.setTokenType(token_type_custom_token);
  }
  else if (Signer.userSigninDataReady())
    Signer.setTokenType(token_type_id_token);

  struct fb_esp_url_info_t uinfo;
  _cfg->_int.fb_auth_uri =
      _cfg->signer.tokens.token_type == token_type_legacy_token ||
      _cfg->signer.tokens.token_type == token_type_id_token;

  if (_cfg->host.length() > 0)
    _cfg->database_url = _cfg->host;

  if (_cfg->database_url.length() > 0)
  {
    ut->getUrlInfo(_cfg->database_url.c_str(), uinfo);
    _cfg->database_url = uinfo.host;
  }

  if (strlen_P(_cfg->cert.data))
    _cfg->_int.fb_caCert = _cfg->cert.data;

  if (_cfg->cert.file.length() > 0)
  {
    if (_cfg->cert.file_storage == mem_storage_type_sd && !_cfg->_int.fb_sd_rdy)
      _cfg->_int.fb_sd_rdy = ut->sdTest(_cfg->_int.fb_file);
    else if ((_cfg->cert.file_storage == mem_storage_type_flash) &&
             !_cfg->_int.fb_flash_rdy)
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
  _auth = auth;
  _cfg = config;

  if (_cfg == nullptr)
    _cfg = &_cfg_;

  if (_auth == nullptr)
    _auth = &_auth_;

  if (ut)
    delete ut;

  ut = new UtilsClass(config);

  RTDB.begin(ut);
  FCM.begin(ut);
  Storage.begin(ut);
  Firestore.begin(ut);
  Functions.begin(ut);
  GCStorage.begin(ut);

  _cfg->_int.fb_reconnect_wifi = WiFi.getAutoReconnect();

  _cfg->signer.signup = false;
  _cfg_.signer.signup = false;
  Signer.begin(ut, _cfg, _auth);
  std::string().swap(_cfg_.signer.tokens.error.message);
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
    _cfg->_int.fb_float_digits = digits;
}

void Firebase_ESP_Client::setDoubleDigits(uint8_t digits)
{
  if (digits < 9)
    _cfg->_int.fb_double_digits = digits;
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