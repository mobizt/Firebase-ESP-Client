/**
 * Mobizt's UDP NTP Time Client, version 1.0.1
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

#ifndef MB_NTP_H
#define MB_NTP_H

#include <Arduino.h>
#include "Udp.h"

class MB_NTP
{

public:
    MB_NTP();

    MB_NTP(UDP *client, const char *host, uint16_t port, int timeZoneOffset = 0);

    ~MB_NTP();

    bool begin();

    bool begin(UDP *client, const char *host, uint16_t port, int timeZoneOffset = 0);

    uint32_t getTime(uint16_t waitMillisec = 0);

private:
    UDP *udp = NULL;
    String host;
    uint16_t port = 0;
    int timeZoneOffset = 0;
    bool udpStarted = false;
    uint16_t intPort = 55432;
    unsigned long timeout = 2000;
    unsigned long lastRequestMs = 0;
    uint32_t ts = 0;
    const uint8_t ntpPacketSize = 48;
    uint8_t packet[48];
    bool sendRequest();
    bool getResponse();
};

MB_NTP::MB_NTP()
{
}

MB_NTP::~MB_NTP()
{
}

MB_NTP::MB_NTP(UDP *client, const char *host, uint16_t port, int timeZoneOffset)
{
    begin(client, host, port, timeZoneOffset);
}

bool MB_NTP::begin(UDP *client, const char *host, uint16_t port, int timeZoneOffset)
{
    this->udp = client;
    this->host = host;
    this->port = port;
    this->timeZoneOffset = timeZoneOffset;

    return this->begin();
}

bool MB_NTP::begin()
{
    if (!this->udp || this->host.length() == 0 || this->port == 0)
        return false;

    udpStarted = udp->begin(intPort) > 0;

    return udpStarted;
}

bool MB_NTP::sendRequest()
{
    if (!udpStarted)
        return false;

    if (lastRequestMs == 0 || millis() - lastRequestMs > timeout)
    {
        lastRequestMs = millis();

        if (!udp->beginPacket(host.c_str(), port))
            return false;

        memset(packet, 0, ntpPacketSize);

        // https://datatracker.ietf.org/doc/html/rfc5905
        packet[0] = 0b11100011; // leap indicator[0-1], version number[2-4], mode[5-7]
        packet[1] = 0;          // stratum 0 is unspecified or invalid
        packet[2] = 6;          // polling interval in log2 seconds
        packet[3] = 236;       // precision in log2 seconds

        // 4 bytes for Root Delay
        // 4 bytes for Root Dispersion
        // 4 bytes for Reference ID (kiss code)
        // 8 bytes for Reference Timestamp
        // 8 bytes for Origin Timestamp
        // 8 bytes for Receive Timestamp
        // 8 bytes for Transmit Timestamp

        if (udp->write(packet, ntpPacketSize) != ntpPacketSize)
            return false;

        if (!udp->endPacket())
            return false;
    }

    return true;
}

bool MB_NTP::getResponse()
{

    if (!udpStarted)
        return false;

    if (!udp->parsePacket())
        return false;

    memset(packet, 0, ntpPacketSize);

    if (udp->read(packet, ntpPacketSize) > 0)
    {
        unsigned long highWord = word(packet[40], packet[41]);
        unsigned long lowWord = word(packet[42], packet[43]);

        unsigned long s1900 = highWord << 16 | lowWord;

        ts = s1900 - 2208988800UL + timeZoneOffset;

        return true;
    }

    return false;
}

uint32_t MB_NTP::getTime(uint16_t waitMillisec)
{

    if (getResponse())
        return ts;

    if (!sendRequest())
        return 0;

    if (waitMillisec > 0)
        delay(waitMillisec);

    if (!getResponse())
        return 0;

    return ts;
}

#endif