
#ifndef SD_HELPER_H_
#define SD_HELPER_H_

#pragma once

#include <Arduino.h>
#include "Firebase.h"

// If SD Card used for storage, assign SD card type and FS used in src/FirebaseFS.h and
// change the config for that card interfaces in this file (src/addons/SDHelper.h)

#if defined(DEFAULT_SD_FS) && defined(CARD_TYPE_SD)

#if defined(ESP32)
#define SPI_CS_PIN 13
#define SPI_SCK_PIN 14
#define SPI_MISO_PIN 2
#define SPI_MOSI_PIN 15
#define SPI_CLOCK_IN_MHz 16
#elif defined(ESP8266)
#define SPI_CS_PIN 15
#endif

// if SdFat library installed and FirebaseFS.h was set to use it (for ESP32 only)
#if defined(USE_SD_FAT_ESP32)

// https://github.com/greiman/SdFat
SdSpiConfig sdFatSPIConfig(SPI_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(SPI_CLOCK_IN_MHz));

#elif defined(ESP32) // if ESP32 and no SdFat library installed

SPIClass spi;

#elif defined(ESP8266)

// SDFSConfig sdFSConfig(SPI_CS_PIN, SPI_HALF_SPEED);

#endif

#endif

bool SD_Card_Mounting()
{

#if defined(DEFAULT_SD_FS) && defined(CARD_TYPE_SD)

    Serial.print("\nMounting SD Card... ");

#if defined(USE_SD_FAT_ESP32)

    if (!Firebase.sdBegin(&sdFatSPIConfig, SPI_CS_PIN, SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN)) // pointer to SdSpiConfig, SS, SCK,MISO, MOSI

#elif defined(ESP32) // if ESP32 and no SdFat library installed

    spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_CS_PIN); // SPI pins config -> SCK,MISO, MOSI, SS
    if (!Firebase.sdBegin(SPI_CS_PIN, &spi))                        // SS, pointer to SPIClass <- SPIClass object should defined as static or global

#elif defined(ESP8266)

    if (!Firebase.sdBegin(SPI_CS_PIN)) // or Firebase.sdBegin(&sdFSConfig)

#endif
    {
        Serial.println("failed\n");
        return false;
    }
    else
    {
        Serial.println("success\n");
        return true;
    }
#endif

#if defined(DEFAULT_SD_FS) && defined(CARD_TYPE_SD_MMC)

    Serial.print("\nMounting SD_MMC Card... ");

    if (!Firebase.sdMMCBegin("/sdcard", false, true))
    {
        Serial.println("failed\n");
        return false;
    }
    else
    {
        Serial.println("success\n");
        return true;
    }
#endif

    return false;
}

#endif
