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
#define DEFAULT_FLASH_FS SPIFFS

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
#include <SD.h>
#define DEFAULT_SD_FS SD
#define CARD_TYPE_SD 1

//For ESP32, format SPIFFS or FFat if mounting failed
#define FORMAT_FLASH_IF_MOUNT_FAILED 1

//Comment to exclude the Firebase Realtime Database
#define ENABLE_RTDB

//Comment to exclude Cloud Firestore
#define ENABLE_FIRESTORE

//Comment to exclude Firebase Cloud Messaging
#define ENABLE_FCM

//Comment to exclude Firebase Storage
#define ENABLE_FB_STORAGE

//Comment to exclude Cloud Storage
#define ENABLE_GC_STORAGE

//Comment to exclude Cloud Function for Firebase
#define ENABLE_FB_FUNCTIONS

/** Use PSRAM for supported ESP32 module */
#if defined(ESP32)
#define FIREBASE_USE_PSRAM
#endif

//tempolary fix the Google server side issue
//https://github.com/mobizt/Firebase-ESP-Client/discussions/165#discussioncomment-1561941
#define FIX_FIRERBASE_RTDB_PRINT_SILENT

#endif