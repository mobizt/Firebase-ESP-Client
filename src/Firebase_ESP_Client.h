#ifndef FIREBASE_CLIENT_VERSION
#define FIREBASE_CLIENT_VERSION "4.3.3"
#endif

/**
 * Google's Firebase ESP Client Main class, Firebase_ESP_Client.h v4.3.3
 *
 * This library supports Espressif ESP8266 and ESP32 MCUs and Raspberry Pi RP2040 Pico MCUs.
 *
 * Created January 19, 2023
 *
 *   Updates:
 * - Fix Firestore incomplete chunked response issue.
 * - Fix chunked response handling issue for authentications.
 * - Fix FCM HTTPv1 invalid message issue.
 * - Fix Storage file openning locked issue.
 * - Fix NTP client issue.
 * - Fix Firebase.ready returns true when network disconnected.
 * - Improve network (WiFi) resume task.
 * - Add support non-ESP device WiFi resume.
 * - Add support SDFS (ESP8266SdFat) filesystem for RP2040/Pico.
 *
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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

#include "Firebase.h"