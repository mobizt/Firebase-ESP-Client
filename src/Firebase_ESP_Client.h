#ifndef FIREBASE_CLIENT_VERSION
#define FIREBASE_CLIENT_VERSION "2.4.0"
#endif

/**
 * Google's Firebase ESP Client Main class, Firebase_ESP_Client.h v2.4.0
 * 
 * This library supports Espressif ESP8266 and ESP32 MCUs
 * 
 * Created August 15, 2021
 * 
*   Updates:
 * 
 * - Code optimization and refactoring.
 * - Bugs fixed e.g. prevent zero length data sent in TCP client.
 * - Major updates with features added and improvement in FirebaseJson, RTDB and Session classes.
 * - Reduce memory usage.
 * - Features added e.g. FirebaseData object's setCert() for locally Root CA verification, to<type>() for any value casting.
 * - RTDB Data type checking is now optional.
 * - Fixed queue management code and file structure.
 * - Rewrite and update examples.
 * - MultipathStream begin and callback functions updated.
 * - FireSense class and example updates. The loadConfig function with default config callback function as argument is set to deprecate.
 * - Searchable Json object and array supported.
 * - FirebaseJsonData casting to any value type is now supported.
 * - FirebaseJson object and array serialize to writable object and deserialize from readable stream and client are supported.
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

#include "Firebase.h"