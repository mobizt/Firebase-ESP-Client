#pragma once

#ifndef FirebaseFS_H
#define FirebaseFS_H

#include <Arduino.h>

#define FIREBASE_ESP_CLIENT 1

/**
 * To use other flash file systems
 *
 * LittleFS File system
 *
 * #include <LittleFS.h>
 * #define DEFAULT_FLASH_FS LittleFS //For ESP8266 LitteFS
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

* The SdFat (https://github.com/greiman/SdFat) is already implemented as wrapper class in ESP8266 core library.
* Do not include SdFat.h library in ESP8266 target code which it conflicts with the wrapper one.

*/

#if defined(ESP32) || defined(ESP8266)
#include <SD.h>
#define DEFAULT_SD_FS SD
#define CARD_TYPE_SD 1
#endif

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

// To enable external Client for ESP8266 and ESP32.
// This will enable automatically for other devices.
// #define FB_ENABLE_EXTERNAL_CLIENT

// For ESP8266 W5100 Ethernet module
// #define ENABLE_ESP8266_W5100_ETH

#endif