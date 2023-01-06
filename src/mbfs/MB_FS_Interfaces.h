
/* Convert specific definitions to MB_FS definitions */
#ifndef MB_FS_INTERFACES_H
#define MB_FS_INTERFACES_H

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// include definitions file
#include "./FirebaseFS.h"

//
#if defined(DEFAULT_FLASH_FS)
#define MBFS_FLASH_FS DEFAULT_FLASH_FS
#endif

//
#if defined(DEFAULT_SD_FS)
#define MBFS_SD_FS DEFAULT_SD_FS
#endif

//
#if defined(CARD_TYPE_SD)
#define MBFS_CARD_TYPE_SD /*  */ CARD_TYPE_SD
#endif

//
#if defined(CARD_TYPE_SD_MMC)
#define MBFS_CARD_TYPE_SD_MMC /*  */ CARD_TYPE_SD_MMC
#endif

//
#if defined(FORMAT_FLASH_IF_MOUNT_FAILED)
#define MBFS_FORMAT_FLASH /*  */ FORMAT_FLASH_IF_MOUNT_FAILED
#endif

#if defined(MBFS_SD_FS) || defined(MBFS_FLASH_FS)
#define MBFS_USE_FILE_STORAGE
#endif

// Only SdFat library from Bill Greiman
#if defined(ESP32) && defined(SD_FAT_VERSION) && defined(SD_FAT_VERSION_STR) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)
#ifndef MBFS_ESP32_SDFAT_ENABLED
#define MBFS_ESP32_SDFAT_ENABLED
#endif

#ifndef USE_SD_FAT_ESP32
#define USE_SD_FAT_ESP32
#endif
#endif

// Only SdFat library from Bill Greiman
#if !defined(ESP32) && !defined(ESP8266) && !defined(PICO_RP2040) && defined(SD_FAT_VERSION) && defined(SD_FAT_VERSION_STR) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)
#ifndef MBFS_SDFAT_ENABLED
#define MBFS_SDFAT_ENABLED
#endif
#endif

// For MB_String
#if defined(FIREBASE_USE_PSRAM)
#define MB_STRING_USE_PSRAM
#endif

//

#if defined(MBFS_SD_FS)

#if !defined(SD_FS_FILE)

#if defined(MBFS_ESP32_SDFAT_ENABLED)
#define MBFS_SD_FILE SdFile
#else

#if defined(ESP32) || defined(ESP8266) || defined(PICO_RP2040)
#define MBFS_SD_FILE fs::File
#else
#define MBFS_SD_FILE File
#endif

#endif

#else

#define MBFS_SD_FILE SD_FS_FILE

#endif

#endif


#ifndef MB_STRING_INCLUDE_CLASS
#define MB_STRING_INCLUDE_CLASS "json/FirebaseJson.h"
#endif

#include MB_STRING_INCLUDE_CLASS

#endif /* MB_FS_INTERFACES_H */
