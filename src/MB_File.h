/**
 * The MB_File, file wrapper class v1.0.0.
 * 
 * This wrapper class is for SD and Flash file interfaces which support SdFat in ESP32 (//https://github.com/greiman/SdFat)
 * 
 *  Created January 16, 2022
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

#ifndef MB_File_H
#define MB_File_H

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "FirebaseFS.h"

#include "addons/fastcrc/FastCRC.h"

#define FS_NO_GLOBALS
#include <FS.h>

#if defined DEFAULT_FLASH_FS
#define FLASH_FS DEFAULT_FLASH_FS
#endif

#if defined DEFAULT_SD_FS
#define SD_FS DEFAULT_SD_FS
#endif

#if defined(SD_FS) || defined(FLASH_FS)
#define USE_FILE_STORAGE
#endif

#define FORMAT_FLASH FORMAT_FLASH_IF_MOUNT_FAILED

#if defined(FIREBASE_USE_PSRAM)
#define FIREBASEJSON_USE_PSRAM
#endif

#include "./json/FirebaseJson.h"

using namespace mb_string;

#define MB_FILE_ERROR_FILE_IO_ERROR -300
#define MB_FILE_ERROR_FILE_NOT_FOUND -301
#define MB_FILE_ERROR_FLASH_STORAGE_IS_NOT_READY -302
#define MB_FILE_ERROR_SD_STORAGE_IS_NOT_READY -303
#define MB_FILE_ERROR_FILE_STILL_OPENED -304

typedef enum
{
    mb_file_mem_storage_type_undefined,
    mb_file_mem_storage_type_flash,
    mb_file_mem_storage_type_sd
} mb_file_mem_storage_type;

typedef enum
{
    mb_file_open_mode_undefined = -1,
    mb_file_open_mode_read = 0,
    mb_file_open_mode_write,
    mb_file_open_mode_append
} mb_file_open_mode;

#define mbfs_file_type mb_file_mem_storage_type
#define mbfs_flash mb_file_mem_storage_type_flash
#define mbfs_sd mb_file_mem_storage_type_sd
#define mbfs_undefined mb_file_mem_storage_type_undefined

#define mbfs_type (mbfs_file_type)

#if defined(ESP32)

#if defined(SD_FS) && defined(CARD_TYPE_SD) && defined(SD_FAT_VERSION)
#define USE_SD_FAT_ESP32
#elif defined(SD_FS)
#define SD_FILE fs::File
#endif

struct fb_esp_sd_mmc_config_info_t
{
    const char *mountpoint = "";
    bool mode1bit = false;
    bool format_if_mount_failed = false;
};

struct fb_esp_sd_config_info_t
{
    int8_t ss = -1;
    int8_t sck = -1;
    int8_t miso = -1;
    int8_t mosi = -1;

#if defined(USE_SD_FAT_ESP32)
    SdSpiConfig *sdFatSPIConfig = nullptr;
#endif

#if defined(ESP32)

#if defined(SD_FS)
    SPIClass *spiConfig = nullptr;
#endif
    fb_esp_sd_mmc_config_info_t sdMMCConfig;
#endif
};

#endif

#if defined(ESP8266)
struct fb_esp_sd_config_info_t
{
    int8_t ss = -1;
#if defined(ESP8266)
    SDFSConfig *sdFSConfig = nullptr;
#endif
};
#endif

#if defined(USE_SD_FAT_ESP32)

#if defined(SD_FS)
#if !defined(SD_FS_FILE)
#define SD_FS_FILE SdFile
#endif
#endif

#else

#if defined(SD_FS)
#if !defined(SD_FS_FILE)
#define SD_FS_FILE File
#endif
#endif

#endif

class MB_File
{


public:
    MB_File() {}
    ~MB_File() {}

    struct fb_esp_sd_config_info_t sd_config;

    //Assign the SD card interfaces with GPIO pins.
    bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1)
    {
        if (sd_rdy)
            return true;

#if defined(SD_FS) && defined(CARD_TYPE_SD)

        sd_config.ss = ss;
#if defined(ESP32)
        sd_config.sck = sck;
        sd_config.miso = miso;
        sd_config.mosi = mosi;
        SPI.begin(sck, miso, mosi, ss);
        return sdSPIBegin(ss, &SPI);
#elif defined(ESP8266)
        return SD_FS.begin(ss);
#endif

#endif
        return false;
    }

#if defined(ESP32) && defined(SD_FS) && defined(CARD_TYPE_SD)

    //Assign the SD card interfaces with SPIClass object pointer (ESP32 only).
    bool sdSPIBegin(int8_t ss, SPIClass *spiConfig)
    {

        if (sd_rdy)
            return true;

#if defined(ESP32)

        sd_config.ss = ss;

        if (spiConfig)
            sd_config.spiConfig = spiConfig;
        else
            sd_config.spiConfig = &SPI;

#if !defined(USE_SD_FAT_ESP32)
        if (ss > -1)
            sd_rdy = SD_FS.begin(ss, *sd_config.spiConfig);
        else
            sd_rdy = SD_FS.begin();
#endif

#elif defined(ESP8266)

        cfg->_int.sd_config.sck = sck;

        if (ss > -1)
            sd_rdy = SD_FS.begin(ss);
        else
            sd_rdy = SD_FS.begin(SD_CS_PIN);
#endif

        return sd_rdy;
    }

#endif

#if defined(USE_SD_FAT_ESP32)

    //Assign the SD card interfaces with SdSpiConfig object pointer and SPI pins assignment (ESP32 only).
    bool sdFatBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
    {

        if (sd_rdy)
            return true;

        if (sdFatSPIConfig)
        {
            sd_config.sdFatSPIConfig = sdFatSPIConfig;
            sd_config.spiConfig = &SPI;
            sd_config.ss = ss;

            if (ss > -1)
                sd_config.spiConfig->begin(sck, miso, mosi, ss);

            sd_rdy = SD_FS.begin(*sd_config.sdFatSPIConfig);
            return sd_rdy;
        }

        return false;
    }
#endif

#if defined(ESP8266)
    //Assign the SD card interfaces with SDFSConfig object pointer (ESP8266 only).
    bool sdFatBegin(SDFSConfig *sdFSConfig)
    {

        if (sd_rdy)
            return true;

        if (sdFSConfig)
        {
            sd_config.sdFSConfig = sdFSConfig;
            SDFS.setConfig(*sd_config.sdFSConfig);
            sd_rdy = SDFS.begin();
            return sd_rdy;
        }

        return false;
    }
#endif

    //Assign the SD_MMC card interfaces (ESP32 only).
    bool sdMMCBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
    {

        if (sd_rdy)
            return true;

#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)

        sd_config.sdMMCConfig.mountpoint = mountpoint;
        sd_config.sdMMCConfig.mode1bit = mode1bit;
        sd_config.sdMMCConfig.format_if_mount_failed = format_if_mount_failed;

        sd_rdy = SD_FS.begin(mountpoint, mode1bit, format_if_mount_failed);
        return sd_rdy;
#endif
#endif
        return false;
    }

    //Check the mounting status of Flash storage.
    bool flashReady()
    {
#if defined FLASH_FS

        if (flash_rdy)
            return true;

#if defined(ESP32)

#if defined(FORMAT_FLASH)
        flash_rdy = FLASH_FS.begin(true);
#else
        flash_rdy = FLASH_FS.begin();
#endif

#elif defined(ESP8266)
        flash_rdy = FLASH_FS.begin();
#endif

#endif

        return flash_rdy;
    }

    //Check the mounting status of SD storage.
    bool sdReady()
    {
#if defined(SD_FS)

        if (sd_rdy)
            return true;

#if defined(ESP32)

#if defined(CARD_TYPE_SD)

        if (!sd_config.spiConfig)
        {
            if (sd_config.ss > -1)
                SPI.begin(sd_config.sck, sd_config.miso, sd_config.mosi, sd_config.ss);
            sd_config.spiConfig = &SPI;
        }

#if defined(USE_SD_FAT_ESP32)

        if (sd_config.sdFatSPIConfig && !sd_rdy)
            sd_rdy = SD_FS.begin(*sd_config.sdFatSPIConfig);
#else
        if (!sd_rdy)
            sd_rdy = sdSPIBegin(sd_config.ss, sd_config.spiConfig);

#endif

#elif defined(CARD_TYPE_SD_MMC)
        if (!sd_rdy)
            sd_rdy = sdMMCBegin(sd_config.sd_mmc_mountpoint, sd_config.sd_mmc_mode1bit, sd_config.sd_mmc_format_if_mount_failed);
#endif

#elif defined(ESP8266)
        if (!sd_rdy)
        {
            if (sd_config.sdFSConfig)
                sd_rdy = sdFatBegin(sd_config.sdFSConfig);
            else
                sd_rdy = sdBegin(sd_config.ss);
        }
#endif

#endif

        return sd_rdy;
    }

    //Check the mounting status of Flash or SD storage with mb_file_mem_storage_type.
    bool checkStorageReady(mbfs_file_type type)
    {

#if defined(USE_FILE_STORAGE)
        if (type == mbfs_flash)
        {
            if (!flash_rdy)
                flashReady();
            return flash_rdy;
        }
        else if (type == mbfs_sd)
        {
            if (!sd_rdy)
                sdReady();
            return sd_rdy;
        }
#endif

        return false;
    }

    //Open file for read or write with file name, mb_file_mem_storage_type and mb_file_open_mode.
    //return size of file (read) or 0 (write) or negative value for error
    int open(const MB_String &filename, mbfs_file_type type, mb_file_open_mode mode)
    {

#if defined(USE_FILE_STORAGE)

        if (!checkStorageReady(type))
        {
            if (type == mbfs_flash)
                return MB_FILE_ERROR_FLASH_STORAGE_IS_NOT_READY;
            else if (type == mbfs_sd)
                return MB_FILE_ERROR_SD_STORAGE_IS_NOT_READY;
            else
                return MB_FILE_ERROR_FILE_IO_ERROR;
        }

        if (mode == mb_file_open_mode_read)
        {
            if (!existed(filename.c_str(), type))
                return MB_FILE_ERROR_FILE_NOT_FOUND;
        }

        int ret = openFile(filename, type, mode);

        if (ret < 0)
            return ret;

        if (ready(type))
            return ret;

#endif
        return MB_FILE_ERROR_FILE_IO_ERROR;
    }

    //Check if file is already open.
    bool ready(mbfs_file_type type)
    {
#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            return true;
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            return true;
#endif
        return false;
    }

    //Get file for read/write with file name, mb_file_mem_storage_type and mb_file_open_mode.
    int size(mbfs_file_type type)
    {
        int size = 0;

#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            size = fb_flash_fs.size();
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            size = fb_sd_fs.size();
#endif
        return size;
    }

    //Check if file is ready to read/write.
    int available(mbfs_file_type type)
    {
        int available = 0;

#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            available = fb_flash_fs.available();
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            available = fb_sd_fs.available();
#endif
        return available;
    }

    //Read byte array. Return the number of bytes that completed read or negative value for error.
    int read(mbfs_file_type type, uint8_t *buf, size_t len)
    {
        int read = 0;
#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            read = fb_flash_fs.read(buf, len);
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            read = fb_sd_fs.read(buf, len);
#endif
        return read;
    }

    //Write byte array. Return the number of bytes that completed write or negative value for error.
    int write(mbfs_file_type type, uint8_t *buf, size_t len)
    {
        int write = 0;
#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            write = fb_flash_fs.write(buf, len);
#endif
#if defined(SD_FS)

        if (type == mbfs_sd && fb_sd_fs)
            write = fb_sd_fs.write(buf, len);
#endif
        return write;
    }

   //Close file.
    void close(mbfs_file_type type)
    {

#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
        {
            fb_flash_fs.close();
            flash_filename_crc = 0;
            flash_opened = false;
            flash_open_mode = mb_file_open_mode_undefined;
        }
#endif

#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
        {
            fb_sd_fs.close();
            sd_filename_crc = 0;
            sd_opened = false;
            sd_open_mode = mb_file_open_mode_undefined;
        }
#endif
    }

    //Check file existence.
    bool existed(const MB_String &filename, mbfs_file_type type)
    {

#if defined(FLASH_FS)
        if (type == mbfs_flash)
            return FLASH_FS.exists(filename.c_str());
#endif

#if defined(SD_FS)
        if (type == mbfs_sd)
        {
#if defined(USE_SD_FAT_ESP32)
            SdFile file;
            bool ret = file.open(filename.c_str(), O_RDONLY);
            file.close();
            return ret;
#else
            return SD_FS.exists(filename.c_str());
#endif
        }
#endif

        return false;
    }
    
    //Seek to position in file.
    void seek(mbfs_file_type type, int pos)
    {
#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            fb_flash_fs.seek(pos);
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            fb_sd_fs.seek(pos);
#endif
    }

    //Read byte. Return the 1 for completed read or negative value for error.
    int read(mbfs_file_type type)
    {
#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            return fb_flash_fs.read();
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            return fb_sd_fs.read();
#endif
        return -1;
    }

    //Write byte. Return the 1 for completed write or negative value for error.
    int write(mbfs_file_type type, uint8_t v)
    {
#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            return fb_flash_fs.write(v);
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            return fb_sd_fs.write(v);
#endif
        return -1;
    }

    bool remove(const MB_String &filename, mbfs_file_type type)
    {
        if (!existed(filename, type))
            return true;

#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            return FLASH_FS.remove(filename.c_str());
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            return SD_FS.remove(filename.c_str());
#endif
        return false;
    }

//Get the Flash file instance.
#if defined(FLASH_FS)
    fs::File &getFlashFile()
    {
        return fb_flash_fs;
    }
#endif

//Get the SD file instance.
#if defined(SD_FS)
    SD_FS_FILE &getSDFile()
    {
        return fb_sd_fs;
    }
#endif

    //Get name of opened file.
    const char *name(mbfs_file_type type)
    {
#if defined(FLASH_FS)
        if (type == mbfs_flash && fb_flash_fs)
            return flash_file.c_str();
#endif
#if defined(SD_FS)
        if (type == mbfs_sd && fb_sd_fs)
            return sd_file.c_str();
#endif

        return "";
    }

    //Calculate CRC16 of byte array.
    uint16_t calCRC(const char *buf)
    {
        return CRC16.ccitt((uint8_t *)buf, strlen(buf));
    }
    
    //Free reserved memory at pointer.
    void delP(void *ptr)
    {
        void **p = (void **)ptr;
        if (*p)
        {
            free(*p);
            *p = 0;
        }
    }

    //Allocate memory
    void *newP(size_t len)
    {
        void *p;
        size_t newLen = getReservedLen(len);
#if defined(BOARD_HAS_PSRAM) && defined(FIREBASE_USE_PSRAM)

        if ((p = (void *)ps_malloc(newLen)) == 0)
            return NULL;

#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.setExternalHeap();
#endif

        bool nn = ((p = (void *)malloc(newLen)) > 0);

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.resetHeap();
#endif

        if (!nn)
            return NULL;

#endif
        memset(p, 0, newLen);
        return p;
    }

    size_t getReservedLen(size_t len)
    {
        int blen = len + 1;

        int newlen = (blen / 4) * 4;

        if (newlen < blen)
            newlen += 4;

        return (size_t)newlen;
    }

private:

    FastCRC16 CRC16;
    uint16_t flash_filename_crc = 0;
    uint16_t sd_filename_crc = 0;
    MB_String flash_file, sd_file;
    mb_file_open_mode flash_open_mode = mb_file_open_mode_undefined;
    mb_file_open_mode sd_open_mode = mb_file_open_mode_undefined;
    bool flash_opened = false;
    bool sd_opened = false;
    bool sd_rdy = false;
    bool flash_rdy = false;

#if defined(FLASH_FS)
    fs::File fb_flash_fs;
#endif
#if defined(SD_FS)
    SD_FS_FILE fb_sd_fs;
#endif

    int openFile(const MB_String &filename, mb_file_mem_storage_type type, mb_file_open_mode mode)
    {

#if defined(FLASH_FS)
        if (type == mbfs_flash)
            return openFlashFile(filename, mode);
#endif
#if defined(SD_FS)
        if (type == mbfs_sd)
            return openSDFile(filename, mode);
#endif
        return MB_FILE_ERROR_FILE_IO_ERROR;
    }

    int openSDFile(const MB_String &filename, mb_file_open_mode mode)
    {
        int ret = MB_FILE_ERROR_FILE_IO_ERROR;

#if defined(SD_FS)

        if (mode == mb_file_open_mode_read || mode == mb_file_open_mode_write)
        {
            uint16_t crc = calCRC(filename.c_str());

            if (mode == sd_open_mode && flash_filename_crc == crc && sd_opened) //same sd file opened, leave it
                return MB_FILE_ERROR_FILE_STILL_OPENED;

            if (sd_opened)
                close(mbfs_sd); //sd file opened, close it

            flash_filename_crc = crc;
        }

#if defined(USE_SD_FAT_ESP32)

        if (mode == mb_file_open_mode_read)
        {
            if (fb_sd_fs.open(filename.c_str(), O_RDONLY))
            {
                sd_file = filename;
                sd_opened = true;
                sd_open_mode = mode;
                ret = fb_sd_fs.size();
            }
        }
        else if (mode == mb_file_open_mode_write)
        {
            if (fb_sd_fs.open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND))
            {
                sd_file = filename;
                sd_opened = true;
                sd_open_mode = mode;
                ret = 0;
            }
        }

#elif defined(SDFileSystem)

        if (mode == mb_file_open_mode_read)
        {
            fb_sd_fs = SD_FS.open(filename.c_str(), FILE_READ);
            if (fb_sd_fs)
            {
                sd_file = filename;
                sd_opened = true;
                sd_open_mode = mode;
                ret = fb_sd_fs.size();
            }
        }
        else if (mode == mb_file_open_mode_write)
        {
            fb_sd_fs = SD_FS.open(filename.c_str(), FILE_WRITE);
            if (fb_sd_fs)
            {
                sd_file = filename;
                sd_opened = true;
                sd_open_mode = mode;
                ret = 0;
            }
        }
#endif

#endif
        return ret;
    }

    int openFlashFile(const MB_String &filename, mb_file_open_mode mode)
    {
        int ret = MB_FILE_ERROR_FILE_IO_ERROR;

#if defined(FLASH_FS)

        if (mode == mb_file_open_mode_read || mode == mb_file_open_mode_write)
        {
            uint16_t crc = calCRC(filename.c_str());
            if (mode == flash_open_mode && sd_filename_crc == crc && flash_opened) //same flash file opened, leave it
                return MB_FILE_ERROR_FILE_STILL_OPENED;

            if (flash_opened)
                close(mbfs_flash); //flash file opened, close it

            sd_filename_crc = crc;
        }

        if (mode == mb_file_open_mode_read)
        {
            fb_flash_fs = FLASH_FS.open(filename.c_str(), "r");
            if (fb_flash_fs)
            {
                flash_file = filename;
                flash_opened = true;
                flash_open_mode = mode;
                ret = fb_flash_fs.size();
            }
        }
        else if (mode == mb_file_open_mode_write)
        {
            fb_flash_fs = FLASH_FS.open(filename.c_str(), "w");
            if (fb_flash_fs)
            {
                flash_file = filename;
                flash_opened = true;
                flash_open_mode = mode;
                ret = 0;
            }
        }

#endif
        return ret;
    }

    
};

#endif
