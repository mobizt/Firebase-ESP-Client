#ifndef FIREBASE_CLIENT_VERSION
#define FIREBASE_CLIENT_VERSION "2.8.2"
#endif

/**
 * Google's Firebase ESP Client Main class, Firebase_ESP_Client.h v2.8.2
 * 
 * This library supports Espressif ESP8266 and ESP32 MCUs
 * 
 * Created January 20, 2022
 *
 *   Updates:
 * - Fixed ESP32 FirebaseJson PSRAM issue.
 * - Fixed download file and download file OTA issues in Firebase Storage and Google Cloud Storage.
 * - Fixed upload file resumable issue in Google Cloud Storage.
 * - Fixed internal url encoding issue.
 * - Fixed flash string handler issue.
 * - Fixed FCM legacy API issue.
 * - Fixed authentication issues.
 * - Fixed issue #231 for FirebaseData object's StringData
 * - Improve storage management.
 * - Add support SdFat in ESP32.
 * - Add support download and upload callback functions for all file operations.
 * - Fixed compilation error of v2.8.0 in ESP32 on Arduino IDE. 
 * 
 * 
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

#include "Firebase.h"