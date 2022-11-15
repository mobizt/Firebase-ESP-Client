/**
 *
 * The Mobizt ESP8266 SSL Client Class, MB_ESP8266_SSLClient.cpp v1.0.1
 *
 * Created November 15, 2022
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

#ifndef MB_ESP8266_SSLCLIENT_CPP
#define MB_ESP8266_SSLCLIENT_CPP

#ifdef ESP8266

#define LWIP_INTERNAL

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "MB_ESP8266_SSLClient.h"
#include <lwip/tcp.h>
#include <include/ClientContext.h>

MB_ESP8266_SSLClient::MB_ESP8266_SSLClient()
{
}

MB_ESP8266_SSLClient::~MB_ESP8266_SSLClient()
{
  stop();
  WCS_CLASS::setClient(nullptr);
  _basic_client = nullptr;
}

void MB_ESP8266_SSLClient::setClient(Client *client)
{
  _basic_client = client;
  WCS_CLASS::setClient(client);
}

int MB_ESP8266_SSLClient::connect(const char *name, uint16_t port)
{

  if (!_basic_client)
    return 0;

  _host = name;

  if (!WCS_CLASS::connect(name, port))
  {
    WCS_CLASS::stop();
    return 0;
  }

  bool res = WCS_CLASS::_connectSSL(_host.c_str());

  if (!res)
    WCS_CLASS::stop();

  return res;
}

int MB_ESP8266_SSLClient::connect(IPAddress ip, uint16_t port)
{
  if (!_basic_client)
    return 0;

  if (!WCS_CLASS::connect(ip, port))
  {
    WCS_CLASS::stop();
    return 0;
  }

  bool res = WCS_CLASS::_connectSSL(nullptr);

  if (!res)
    WCS_CLASS::stop();

  return res;
}

bool MB_ESP8266_SSLClient::connectSSL()
{

  bool res = WCS_CLASS::_connectSSL(_host.c_str());

  if (!res)
    WCS_CLASS::stop();

  return res;
}

uint8_t MB_ESP8266_SSLClient::connected()
{
  return WCS_CLASS::connected();
}

void MB_ESP8266_SSLClient::setTimeout(unsigned long timeout)
{
  WCS_CLASS::setTimeout(timeout);
}

void MB_ESP8266_SSLClient::stop()
{
  WCS_CLASS::stop();
}

int MB_ESP8266_SSLClient::available()
{
  return WCS_CLASS::available();
}

int MB_ESP8266_SSLClient::read()
{
  return WCS_CLASS::read();
}

int MB_ESP8266_SSLClient::read(uint8_t *buf, size_t size)
{
  return WCS_CLASS::read(buf, size);
}

size_t MB_ESP8266_SSLClient::write(const uint8_t *buf, size_t size)
{
  return WCS_CLASS::write(buf, size);
}

int MB_ESP8266_SSLClient::peek()
{
  return WCS_CLASS::peek();
}

void MB_ESP8266_SSLClient::setInSecure()
{
  WCS_CLASS::setInsecure();
}

void MB_ESP8266_SSLClient::flush()
{
  WCS_CLASS::flush();
}

void MB_ESP8266_SSLClient::setBufferSizes(int recv, int xmit)
{
  WCS_CLASS::setBufferSizes(recv, xmit);
}

int MB_ESP8266_SSLClient::availableForWrite()
{
  return WCS_CLASS::availableForWrite();
}

void MB_ESP8266_SSLClient::setSession(BearSSL_Session *session)
{
  WCS_CLASS::setSession(session);
}

void MB_ESP8266_SSLClient::setKnownKey(const BearSSL_PublicKey *pk, unsigned usages)
{
  WCS_CLASS::setKnownKey(pk, usages);
}

bool MB_ESP8266_SSLClient::setFingerprint(const uint8_t fingerprint[20])
{
  return WCS_CLASS::setFingerprint(fingerprint);
}

bool MB_ESP8266_SSLClient::setFingerprint(const char *fpStr)
{
  return WCS_CLASS::setFingerprint(fpStr);
}

void MB_ESP8266_SSLClient::allowSelfSignedCerts()
{
  WCS_CLASS::allowSelfSignedCerts();
}

void MB_ESP8266_SSLClient::setTrustAnchors(const BearSSL_X509List *ta)
{
  WCS_CLASS::setTrustAnchors(ta);
}

void MB_ESP8266_SSLClient::setX509Time(time_t now)
{
  WCS_CLASS::setX509Time(now);
}

void MB_ESP8266_SSLClient::setClientRSACert(const BearSSL_X509List *cert, const BearSSL_PrivateKey *sk)
{
  WCS_CLASS::setClientRSACert(cert, sk);
}

void MB_ESP8266_SSLClient::setClientECCert(const BearSSL_X509List *cert, const BearSSL_PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type)
{
  WCS_CLASS::setClientECCert(cert, sk, allowed_usages, cert_issuer_key_type);
}

int MB_ESP8266_SSLClient::getMFLNStatus()
{
  return WCS_CLASS::getMFLNStatus();
}

int MB_ESP8266_SSLClient::getLastSSLError(char *dest, size_t len)
{
  return WCS_CLASS::getLastSSLError(dest, len);
}

void MB_ESP8266_SSLClient::setCertStore(BearSSL_CertStoreBase *certStore)
{
  WCS_CLASS::setCertStore(certStore);
}

bool MB_ESP8266_SSLClient::setCiphers(const uint16_t *cipherAry, int cipherCount)
{
  return WCS_CLASS::setCiphers(cipherAry, cipherCount);
}

bool MB_ESP8266_SSLClient::setCiphers(const std::vector<uint16_t> &list)
{
  return WCS_CLASS::setCiphers(list);
}

bool MB_ESP8266_SSLClient::setCiphersLessSecure()
{
  return WCS_CLASS::setCiphersLessSecure();
}

bool MB_ESP8266_SSLClient::setSSLVersion(uint32_t min, uint32_t max)
{
  return WCS_CLASS::setSSLVersion(min, max);
}

const char *MB_ESP8266_SSLClient::peekBuffer()
{
  return WCS_CLASS::peekBuffer();
}

void MB_ESP8266_SSLClient::peekConsume(size_t consume)
{
  WCS_CLASS::peekConsume(consume);
}

#endif /* ESP8266 */

#endif /* MB_ESP8266_SSLCLIENT_CPP */