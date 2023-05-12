#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

#ifndef FirebaseFS_H
#define FirebaseFS_H

#include <Arduino.h>
#include "mbfs/MB_MCU.h"

#define FIREBASE_ESP_CLIENT 1

#define FB_DEFAULT_DEBUG_PORT Serial

/**
 * To use other flash file systems
 *
 * LittleFS File system
 *
 * #include <LittleFS.h>
 * #define DEFAULT_FLASH_FS LittleFS //For ESP8266 or RPI2040 LitteFS
 *
 *
 * FAT File system
 *
 * #include <FFat.h>
 * #define DEFAULT_FLASH_FS FFat  //For ESP32 FAT
 *
 */
#if defined(ESP32)
#include <SPIFFS.h>
#endif
#if defined(ESP32) || defined(ESP8266)
#define DEFAULT_FLASH_FS SPIFFS
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_NANO_RP2040_CONNECT)
#include <LittleFS.h>
#define DEFAULT_FLASH_FS LittleFS
#endif

/**
 * To use SD card file systems with different hardware interface
 * e.g. SDMMC hardware bus on the ESP32
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD#faq
 *
 #include <SD_MMC.h>
 #define DEFAULT_SD_FS SD_MMC //For ESP32 SDMMC
 #define CARD_TYPE_SD_MMC 1 //For ESP32 SDMMC

 *
*/

/**
 * To use SdFat on ESP32

#if defined(ESP32)
#include <SdFat.h> // https://github.com/greiman/SdFat
static SdFat sd_fat_fs;   // should declare as static here
#define DEFAULT_SD_FS sd_fat_fs
#define CARD_TYPE_SD 1
#define SD_FS_FILE SdFile
#endif

* The SdFat (https://github.com/greiman/SdFat) is already implemented as wrapper class in ESP8266 and RP2040 core libraries.
* Do not include SdFat.h library in ESP8266 and RP2040 target codes which it conflicts with the wrapper one.

*/

#if defined(ESP32) || defined(ESP8266)
#include <SD.h>
#define DEFAULT_SD_FS SD
#define CARD_TYPE_SD 1
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_NANO_RP2040_CONNECT)
// Use SDFS (ESP8266SdFat) instead of SD
#include <SDFS.h>
#define DEFAULT_SD_FS SDFS
#define CARD_TYPE_SD 1
#endif

// For RTDB legacy token usage only
// #define USE_LEGACY_TOKEN_ONLY

// Enable the error string from fbdo.errorReason */
// You can get the error code from fbdo.errorCode() when disable this option
#define ENABLE_ERROR_STRING

// For ESP32, format SPIFFS or FFat if mounting failed
#define FORMAT_FLASH_IF_MOUNT_FAILED 1

// Comment to exclude the Firebase Realtime Database
#define ENABLE_RTDB

#define ENABLE_ERROR_QUEUE

// Comment to exclude Cloud Firestore
#define ENABLE_FIRESTORE

// Comment to exclude Firebase Cloud Messaging
#define ENABLE_FCM

// Comment to exclude Firebase Storage
#define ENABLE_FB_STORAGE

// Comment to exclude Cloud Storage
#define ENABLE_GC_STORAGE

// Comment to exclude Cloud Function for Firebase
#define ENABLE_FB_FUNCTIONS

/** Use PSRAM for supported ESP32/ESP8266 module */
#if defined(ESP32) || defined(ESP8266)
#define FIREBASE_USE_PSRAM
#endif

// To enable OTA updates via RTDB, Firebase Storage and Google Cloud Storage buckets
#define ENABLE_OTA_FIRMWARE_UPDATE

// Use Keep Alive connection mode
#define USE_CONNECTION_KEEP_ALIVE_MODE

// To enable external Client for ESP8266, ESP32 and Raspberry Pi Pico.
// This will enable automatically for other devices.
// #define FB_ENABLE_EXTERNAL_CLIENT

// For ESP8266 ENC28J60 Ethernet module
// #define ENABLE_ESP8266_ENC28J60_ETH

// For ESP8266 W5100 Ethernet module
// #define ENABLE_ESP8266_W5100_ETH

// For ESP8266 W5500 Ethernet module
// #define ENABLE_ESP8266_W5500_ETH

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
// You can create your own header file "CustomFirebaseFS.h" in the same diectory of
// "FirebaseFS.h" and put your own custom config to overwrite or
// change the default config in "FirebaseFS.h".
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

/** This is an example of "CustomFirebaseFS.h"

#pragma once

#ifndef CustomFirebaseFS_H
#define CustomFirebaseFS_H

// Use external client instead of internal client
#define FB_ENABLE_EXTERNAL_CLIENT // define to use external client

// Use LittleFS instead of SPIFFS
#include "LittleFS.h"
#undef DEFAULT_FLASH_FS // remove Flash FS defined macro
#define DEFAULT_FLASH_FS LittleFS

// Use SD_MMC instead of SD
#if defined(ESP32)
#include <SD_MMC.h>
#undef DEFAULT_SD_FS // remove SD defined macro
#undef CARD_TYPE_SD // remove SD defined macro
#define DEFAULT_SD_FS SD_MMC
#define CARD_TYPE_SD_MMC 1
#endif

// Disable Error Queue, Firestore, FCM, Firebase Storage, Google Cloud Storage
// and Functions for Firebase.
#undef ENABLE_ERROR_QUEUE
#undef ENABLE_FIRESTORE
#undef ENABLE_FCM
#undef ENABLE_FB_STORAGE
#undef ENABLE_GC_STORAGE
#undef ENABLE_FB_FUNCTIONS

#endif

*/
#if __has_include("CustomFirebaseFS.h")
#include "CustomFirebaseFS.h"
#endif

#endif

/////////////////////////////////// WARNING ///////////////////////////////////
// Using RP2040 Pico Arduino SDK, FreeRTOS with LittleFS will cause device hangs
// when write the data to flash filesystem.
// Do not include free rtos dot h or even it excluded from compilation by using macro
// or even comment it out with "//"".