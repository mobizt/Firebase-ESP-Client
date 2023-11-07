# Firebase Arduino Client Library for for Arduino devices

![Compile](https://github.com/mobizt/Firebase-ESP-Client/actions/workflows/compile_library.yml/badge.svg) ![Examples](https://github.com/mobizt/Firebase-ESP-Client/actions/workflows/compile_examples.yml/badge.svg)  [![Github Stars](https://img.shields.io/github/stars/mobizt/Firebase-ESP-Client?logo=github)](https://github.com/mobizt/Firebase-ESP-Client/stargazers) ![Github Issues](https://img.shields.io/github/issues/mobizt/Firebase-ESP-Client?logo=github)

![arduino-library-badge](https://www.ardu-badge.com/badge/Firebase%20Arduino%20Client%20Library%20for%20ESP8266%20and%20ESP32.svg) ![PlatformIO](https://badges.registry.platformio.org/packages/mobizt/library/Firebase%20Arduino%20Client%20Library%20for%20ESP8266%20and%20ESP32.svg)


This library provides Firebase Realtime database, Firebase Cloud Messaging, Cloud Firestore database, Firebase Storage, Google Cloud Storage and Cloud Functions for Firebase functions and supports most Arduino devices except for AVR devices that have resources and compiler limit.

This library can work with on-chip/on-board network (WiFi/Ethernet) and External network module via Client library.

The features can be configurable to add and exclude some unused features, see [Library Build Options](#library-build-options).

The [ESP8266 and Raspberry Pi Pico](https://github.com/mobizt/Firebase-ESP8266) and [ESP32](https://github.com/mobizt/Firebase-ESP32) versions are available which provide only Firebase Realtime database and Firebase Cloud Messaging functions.

Try the beta version of new [AsyncFirebaseClient](https://github.com/mobizt/AsyncFirebaseClient) library which is faster and more reliable.

The new [AsyncFirebaseClient](https://github.com/mobizt/AsyncFirebaseClient) is currently support Realtime Database.

## Contents

[1. Features](#features)

[2. Breaking Changes from version 4](#breaking-changes-from-version-4)

[3. Supported Devices](#supported-devices)

[4. Dependencies](#dependencies)

[5. Installation](#installation)

- [Using Library Manager](#using-library-manager)

- [Manual installation](#manual-installation)

- [RP2040 Arduino SDK installation](#rp2040-arduino-sdk-installation)

[6. Usages](#usages)

- [Initialization](#initialization)

[7. IDE Build Options](#ide-build-options)

- [Memory Options for ESP8266](#memory-options-for-esp8266)

- [Arduino IDE](#arduino-ide)

- [PlatformIO IDE](#platformio-ide)

- [ESP8266 and SRAM/PSRAM Chip connection](#esp8266-and-srampsram-chip-connection)

- [Memory Options for ESP32](#memory-options-for-esp32)

- [Arduino IDE](#arduino-ide-1)

- [PlatformIO IDE](#platformio-ide-1)

[8. Library Build Options](#library-build-options)

- [Predefined Options](#predefined-options)

- [Optional Options](#optional-options)

[9. TCP Client Options](#tcp-client-options)

- [TCP Keep Alive](#tcp-keep-alive)

- [TCP Time Out](#tcp-time-out)

[10. Firebase Authentications Options](#firebase-authentications-options)

- [Access in Test Mode (No Auth)](#access-in-test-mode-no-auth)

- [The authenication credentials and prerequisites](#the-authenication-credentials-and-prerequisites)


[11. Realtime Database](#realtime-database)

- [Read Data](#read-data)

- [Store Data](#store-data)

- [Append Data](#append-data)

- [Patch Data](#patch-data)

- [Delete Data](#delete-data)

- [Filtering Data](#filtering-data)

- [Monitoring data](#monitoring-data)

- [Enable TCP KeepAlive for reliable HTTP Streaming](#enable-tcp-keepalive-for-reliable-http-streaming)

- [HTTP Streaming examples](#http-streaming-examples)

[12. Firebase Cloud Messaging (FCM)](#firebase-cloud-messaging-fcm)

[13. Firebase Cloud Firestore](#firebase-cloud-firestore)

[14. Firebase Storage](#firebase-storage)

[15. Google Cloud Storage](#google-cloud-storage)

[16. Cloud Functions for Firebase](#cloud-functions-for-firebase)

- [IAM Permission and API Enable](#iam-permission-and-api-enable)

[17. Create, Edit, Serializing and Deserializing the JSON Objects](#create-edit-serializing-and-deserializing-the-json-objects)

[18. Acheivement](#acheivement)

- [Open Sourcs Contribution Awards](#open-sourcs-contribution-awards)

[19. License](#license)


## Features

* Supports most Arduino devices (except for AVR) with or without external neywork module.

* Supports most Firebase Services included Google Cloud Storage.

* Supports external Heap using SRAM/PSRAM in ESP8266 and ESP32.

* TinyGSMClient and Ethernet Client integration.

* Faster server reconnection with SSL Session Resumption.

* Supports Authentications and Test Mode (No Auth for some services).

* Supports Firmware OTA updates.

## Breaking Changes from version 4

To keep support only necessary options and features that provided by Firebase which can be reduce the program footprint, there are some functions removed from this library in version 5.

Note that, the following changes applied for [Firebase-ESP-Client](https://github.com/mobizt/Firebase-ESP-Client),  [Firebase-ESP8266](https://github.com/mobizt/Firebase-ESP8266) and [Firebase-ESP32](https://github.com/mobizt/Firebase-ESP32) libraries.

### The functions removal.

- The MultiPath Streaming in RTDB (no replacement).

  `beginMultiPathStream`, `setMultiPathStreamCallback`, and `removeMultiPathStreamCallback`
  
- The Streaming in RTDB (with replacement).
  
  `beginStream`, `endStream`, `setStreamCallback` and `removeStreamCallback`

  Note that, use `stream` and `stopStream` instead of `beginStream` and `endStream`.

- The backup and restore in RTDB (no replacement).

  `backup`, `restore`, `getBackupFilename`, and `getBackupFileSize`

- The error queue and its data type in RTDB (no replacement).

  `setMaxErrorQueue`, `saveErrorQueue`, `deleteStorageFile`, `restoreErrorQueue`, `errorQueueCount`, `isErrorQueueFull`, `processErrorQueue`, `getErrorQueueID`, `isErrorQueueExisted`, `beginAutoRunErrorQueue`, `endAutoRunErrorQueue`, and `clearErrorQueue`

- The security rules download, upload and set the read/write rules in RTDB (no replacement).

   `getRules`, `setRules` and `setReadWriteRules`.

- The database security rules's query index set/remove in RTDB (no replacement).

  `setQueryIndex` and `removeQueryIndex`

- The anync functions in RTDB (no replacement but alternative).
  
  `setPriorityAsync`, `pushAsync`, `pushIntAsync`, `pushFloatAsync`, `pushDoubleAsync`, `pushBoolAsync`, `pushStringAsync`, `pushJSONAsync`, `pushArrayAsync`, `pushBlobAsync`, `pushFileAsync`, `pushTimestampAsync`, `setAsync`, `setIntAsync`, `setFloatAsync`, `setDoubleAsync`, `setDoubleAsync`, `setBoolAsync`, `setStringAsync`, `setJSONAsync`, `setArrayAsyn`, `setBlobAsync`, `setFileAsync`, `setTimestampAsync`, `updateNodeAsync`, and `updateNodeSilentAsync`

  Note that, see [Store Data](#store-data), [Append Data](#append-data) and [Patch Data](#patch-data) topics for alternative.

- The node deletion in RTDB (with replacement).

  `deleteNode` and `deleteNodesByTimestamp`

  Note that, use `remove` for `deleteNode` instead. No alternative function of `deleteNodesByTimestamp` is available.

- Old API in FCM for [Firebase-ESP8266](https://github.com/mobizt/Firebase-ESP8266) and [Firebase-ESP32](https://github.com/mobizt/Firebase-ESP32) libraries (no replacement).

  `fcm.begin`, `fcm.addDeviceToken`, `fcm.removeDeviceToken`, `fcm.clearDeviceToken`, `fcm.setNotifyMessage`, `fcm.addCustomNotifyMessage`, `fcm.setDataMessage`, `fcm.clearDataMessage`, `fcm.setPriority`, `fcm.setCollapseKey`, `fcm.setTimeToLive`, `fcm.setTopic`, `fcm.getSendResult`, `sendMessage`, `broadcastMessage` and `sendTopic`

- The async function in Cloud Firestore (no replacement).

  `commitDocumentAsync`

- The functions getXXX, pushXXX, and setXXX in RTDB (with replacement)
  
  `pushInt`, `pushFloat`, `pushDouble`, `pushBool`, `pushString`, `pushJSON`, `pushArray`, `pushBlob`, `pushFile`, `setInt`, `setFloat`, `setDouble`, `setBool`, `setString`, `setJSON`, `setArray`, `setBlob`, `setFile`,`getInt`, `getFloat`, `getDouble`, `getBool`, `getString`,`getJSON`, `getArray`, `getBlob`, `getFile`.

  Note that, use generic `get`,`push`, and `set` instead.

- The update (patch) functions in RTDB (with replacement).

  `updateNode` and `updateNodeSilent`

  Note that, use `update` instead.

- Old API in RTDB for [Firebase-ESP8266](https://github.com/mobizt/Firebase-ESP8266) and [Firebase-ESP32](https://github.com/mobizt/Firebase-ESP32) libraries (with replacement).

  `setReadTimeout`, `setwriteSizeLimit`, `pathExisted`, `pathExist`, `getETag`, `getShallowData`, `enableClassicRequest`, `setPriority`, `getPriority`, `push`, `pushInt`, `pushFloat`, `pushDouble`, `pushBool`, `pushString`, `pushJSON`, `pushArray`, `pushBlob`, `pushFile`, `pushTimestamp`, `setInt`, `setFloat`, `setDouble`, `setBool`, `setString`, `setJSON`, `setArray`, `setBlob`, `setFile`, `setTimestamp`, `updateNode`, `updateNodeSilent`, `get`, `getInt`, `getFloat`, `getDouble`, `getBool`, `getString`, `getJSON`, `getArray`, `getBlob`, `getFile`, `downloadOTA`, `deleteNode`, `beginStream`, `readStream`, `runStream`, and `setMaxRetry`

  Note that these functions should be called from `RTDB` member class of `Firebase` class and use the pointer to `FirebaseData`, `FirebaseJson` and `FirebaseJsonArray` in the functions.

- The legacy functions in `FirebaseData` (with replacement).

  `intData`, `floatData`, `doubleData`, `boolData`, `stringData`, `jsonString`, `jsonObject`, `jsonObjectPtr`, `jsonArray`, `jsonArrayPtr`, `blobData`, and `fileStream`

- The external Client functions (with replacement).

  `setExternalClient` and `setExternalClientCallbacks`

- The FireSense add on in RTDB (no replacement).

  `loadConfig`, `backupConfig`, `restoreConfig`, `enableController`, `addChannel`, `addCondition`, `addCallbackFunction`, `clearAllCallbackFunctions`, `addUserValue`, `clearAllUserValues`, and `getDeviceId`

### The classes removal.

- The MultiPath Streaming in RTDB (no replacement).

   `MultiPathStream`

- The Streaming in RTDB (no replacement).

  `FirebaseStream` for [Firebase-ESP-Client](https://github.com/mobizt/Firebase-ESP-Client) and `StreamData` for [Firebase-ESP8266](https://github.com/mobizt/Firebase-ESP8266) and [Firebase-ESP32](https://github.com/mobizt/Firebase-ESP32) libraries.

- The error queue in RTDB (no replacement).

  `QueueManager` and `QueueInfo`

- FireSense add on in RTDB (no replacement).
  
  `FireSenseClass`

### The enums and structs removal

  For naming consitency across the library, the followings are removed.

- The `fb_esp_rtdb_data_type` enums in RTDB (with replacement).

  `fb_esp_rtdb_data_type_null`, `fb_esp_rtdb_data_type_integer`, `fb_esp_rtdb_data_type_float`, `fb_esp_rtdb_data_type_double`, `fb_esp_rtdb_data_type_boolean`, `fb_esp_rtdb_data_type_string`, `fb_esp_rtdb_data_type_json`, `fb_esp_rtdb_data_type_array`, `fb_esp_rtdb_data_type_blob`, and `fb_esp_rtdb_data_type_file`

- The `fb_esp_rtdb_upload_status` enums in RTDB (with replacement).
  
  `fb_esp_rtdb_upload_status_error`, `fb_esp_rtdb_upload_status_unknown`, `fb_esp_rtdb_upload_status_init` `fb_esp_rtdb_upload_status_upload`, and `fb_esp_rtdb_upload_status_complete`

- The `fb_esp_rtdb_download_status` enums in RTDB (with replacement).
  
  `fb_esp_rtdb_download_status_error`, `fb_esp_rtdb_download_status_unknown`, `fb_esp_rtdb_download_status_init` `fb_esp_rtdb_download_status_download`,and `fb_esp_rtdb_download_status_complete`

- The `fb_esp_cfs_upload_status` enums in Cloud Firestore (with replacement).
  
  `fb_esp_cfs_upload_status_error`, `fb_esp_cfs_upload_status_unknown`, `fb_esp_cfs_upload_status_init` `fb_esp_cfs_upload_status_upload`,`fb_esp_cfs_upload_status_complete`, and `fb_esp_cfs_upload_status_process_response`

- The `fb_esp_gcs_upload_status` enums in Google Cloud Storage (with replacement).
  
  `fb_esp_gcs_upload_status_error`, `fb_esp_gcs_upload_status_unknown`, `fb_esp_gcs_upload_status_init` `fb_esp_gcs_upload_status_upload`, and `fb_esp_gcs_upload_status_complete`

- The `fb_esp_gcs_download_status` enums in Google Cloud Storage (with replacement).
  
  `fb_esp_gcs_download_status_error`, `fb_esp_gcs_download_status_unknown`, `fb_esp_gcs_download_status_init` `fb_esp_gcs_download_status_download`, and `fb_esp_gcs_download_status_complete`

- The `fb_esp_fcs_upload_status` enums in Firebase Storage (with replacement).
  
  `fb_esp_fcs_upload_status_error`, `fb_esp_fcs_upload_status_unknown`, `fb_esp_fcs_upload_status_init` `fb_esp_fcs_upload_status_upload`, and `fb_esp_fcs_upload_status_complete`

- The `fb_esp_fcs_download_status` enums in Firebase Storage (with replacement).
  
  `fb_esp_fcs_download_status_error`, `fb_esp_fcs_download_status_unknown`, `fb_esp_fcs_download_status_init` `fb_esp_fcs_download_status_download`, and `fb_esp_fcs_download_status_complete`

- The `fb_esp_functions_operation_status` enums in Cloud Functions for Firebase (with replacement).
  
  `fb_esp_functions_operation_status_unknown`, `fb_esp_functions_operation_status_generate_upload_url`, `fb_esp_functions_operation_status_upload_source_file_in_progress` `fb_esp_functions_operation_status_deploy_in_progress`, `fb_esp_functions_operation_status_set_iam_policy_in_progress`, `fb_esp_functions_operation_status_delete_in_progress`, `fb_esp_functions_operation_status_finished` and `fb_esp_functions_operation_status_error`

Note that, you can replace the enums `fb_esp_xxx` with `firebase_xxx`.

- The StorageType struct for [Firebase-ESP8266](https://github.com/mobizt/Firebase-ESP8266) and [Firebase-ESP32](https://github.com/mobizt/Firebase-ESP32) libraries (with replacement).
  
  `StorageType::UNDEFINED`, `StorageType::FLASH` and `StorageType::SD`

Note that, `mem_storage_type_undefined`, `mem_storage_type_flash`, and `mem_storage_type_sd` was used instead.

- The error queue in RTDB (no replacement).
  
  `QueueStorageType` and `QueueItem`

- The FireSense add on (no replacement).

  `Firesense_Config`, `FireSense_Channel`, `Firesense_Channel_Type`, `FireSense_Condition`, and `FireSense_Data_Type`

### The platform and its features removal.

Because of no benefit we gain from using `FreeRTOS` in `Raspberry Pi Pico` platform in this library and limited resources and C++ functions supported in `AVR` platform, the following are changes.

- `FreeRTOS` removal in Raspberry Pi Pico in Google Cloud Storage upload task.

- `FreeRTOS` removal in Raspberry Pi Pico in Functions for Firebase task.

  `createFunction` and `runDeployTasks`

- The `std::vecor` was used to represent the dynamic array and it breaks the `AVR` platform supports.


For proper usage guidlines, see [examples](/examples/)


### To bring back the removed functions.
You should copy the relevant functions you used in the source code of version 4 (mostly in `FB_RTDB.h` and `FB_RTDB.cpp`) to your sketch.

The removed functions implemented using the library public functions and some private helper functions.


## Supported Devices.

 * ESP8266 MCUs based boards
 * ESP32 MCUs based boards
 * Arduino MKR WiFi 1010
 * Arduino MKR 1000 WIFI
 * Arduino Nano 33 IoT
 * Arduino MKR Vidor 4000
 * Raspberry Pi Pico (RP2040)
 * Arduino UNO R4 WiFi (Renesas).
 * LAN8720 Ethernet PHY
 * TLK110 Ethernet PHY
 * IP101 Ethernet PHY
 * ENC28J60 SPI Ethernet module
 * W5100 SPI Ethernet module
 * W5500 SPI Ethernet module
 * SIMCom Modules with TinyGSMClient



## Dependencies


This library required **Platform's Core SDK** to be installed.

ESP8266 Core SDK v2.5.0 and older versions are not supported.

For Arduino IDE, ESP8266 Core SDK can be installed through **Boards Manager**. 

For PlatfoemIO IDE, ESP8266 Core SDK can be installed through **PIO Home** > **Platforms** > **Espressif 8266 or Espressif 32**.

The RP2040 boards required Arduino-Pico SDK from Earle F. Philhower https://github.com/earlephilhower/arduino-pico



## Installation


### Using Library Manager

At Arduino IDE, go to menu **Sketch** -> **Include Library** -> **Manage Libraries...**

In Library Manager Window, search **"firebase"** in the search form then select **"Firebase ESP Client"**. 

Click **"Install"** button.



For PlatformIO IDE, using the following command.

**pio lib install "Firebase ESP Client""**

Or at **PIO Home** -> **Library** -> **Registry** then search **Firebase ESP Client**.



### Manual installation

For Arduino IDE, download zip file from the repository (Github page) by select **Code** dropdown at the top of repository, select **Download ZIP** 

From Arduino IDE, select menu **Sketch** -> **Include Library** -> **Add .ZIP Library...**.

Choose **Firebase-ESP-Client-main.zip** that previously downloaded.

Rename **Firebase-ESP-Client-main** folder to **Firebase_Arduino_Client_Library_for_ESP8266_and_ESP32**.

Go to menu **Files** -> **Examples** -> **Firebase-ESP-Client-main** and choose one from examples.



### RP2040 Arduino SDK installation

For Arduino IDE, the Arduino-Pico SDK can be installed from Boards Manager by searching pico and choose Raspberry Pi Pico/RP2040 to install.

For PlatformIO, the Arduino-Pico SDK can be installed via platformio.ini

```ini
[env:rpipicow]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = rpipicow
framework = arduino
board_build.core = earlephilhower
monitor_speed = 115200
board_build.filesystem_size = 1m
```

See this Arduino-Pico SDK [documentation](https://arduino-pico.readthedocs.io/en/latest/) for more information.


## Usages


See [all examples](/examples) for complete usages.

See [function description](/src/README.md) for all available functions.

See [other authentication examples](/examples/Authentications) for more sign in methods.



## IDE Build Options

### Memory Options for ESP8266

When you update the ESP8266 Arduino Core SDK to v3.0.0, the memory can be configurable from IDE.

You can choose the Heap memory between internal and external memory chip from IDE e.g. Arduino IDE and PlatformIO on VSCode or Atom IDE.

#### Arduino IDE


For ESP8266 devices that don't have external SRAM/PSRAM chip installed, choose the MMU **option 3**, 16KB cache + 48KB IRAM and 2nd Heap (shared).

![Arduino IDE config](/media/images/ArduinoIDE.png)

For ESP8266 devices that have external 23LC1024 SRAM chip installed, choose the MMU **option 5**, 128K External 23LC1024.

![MMU VM 128K](/media/images/ESP8266_VM.png)

For ESP8266 devices that have external ESP-PSRAM64 chip installed, choose the MMU **option 6**, 1M External 64 MBit PSRAM.


#### PlatformIO IDE

The MMU options can be selected from build_flags in your project's platformio.ini file

For ESP8266 devices that don't not have external SRAM/PSRAM chip installed, add build flag as below.

```ini
[env:d1_mini]
platform = espressif8266
build_flags = -D PIO_FRAMEWORK_ARDUINO_MMU_CACHE16_IRAM48_SECHEAP_SHARED
board = d1_mini
framework = arduino
monitor_speed = 115200
```


For ESP8266 devices that have external 23LC1024 SRAM chip installed, add build flag as below.

```ini
[env:d1_mini]
platform = espressif8266
;128K External 23LC1024
build_flags = -D PIO_FRAMEWORK_ARDUINO_MMU_EXTERNAL_128K
board = d1_mini
framework = arduino
monitor_speed = 115200
```


For ESP8266 devices that have external ESP-PSRAM64 chip installed, add build flag as below.

```ini
[env:d1_mini]
platform = espressif8266
;1M External 64 MBit PSRAM
build_flags = -D PIO_FRAMEWORK_ARDUINO_MMU_EXTERNAL_1024K
board = d1_mini
framework = arduino
monitor_speed = 115200
```


#### ESP8266 and SRAM/PSRAM Chip connection

Most ESP8266 modules don't have the built-in SRAM/PSRAM on board. External memory chip connection can be done via SPI port as below.

```
23LC1024/ESP-PSRAM64                ESP8266

CS (Pin 1)                          GPIO15
SCK (Pin 6)                         GPIO14
MOSI (Pin 5)                        GPIO13
MISO (Pin 2)                        GPIO12
/HOLD (Pin 7 on 23LC1024 only)      3V3
Vcc (Pin 8)                         3V3
Vcc (Pin 4)                         GND
```

Once the external Heap memory was selected in IDE, to allow the library to use the external memory, you can set it in [**FirebaseFS.h**](src/FirebaseFS.h) by define this macro.


```cpp
#define FIREBASE_USE_PSRAM
```

This macro was defined by default when you installed or update the library.



### Memory Options for ESP32

In ESP32 module that has PSRAM installed, you can enable it and set the library to use this external memory instead.

#### Arduino IDE

To enable PSRAM in ESP32 module.

![Enable PSRAM in ESP32](/media/images/ESP32-PSRAM.png)


#### PlatformIO IDE


In PlatformIO on VSCode or Atom IDE, add the following build_flags in your project's platformio.ini file.

```ini
build_flags = -DBOARD_HAS_PSRAM -mfix-esp32-psram-cache-issue
```

As in ESP8266, once the external Heap memory was enabled in IDE, to allow the library to use the external memory, you can set it in [**FirebaseFS.h**](src/FirebaseFS.h) by define this macro.

```cpp
#define FIREBASE_USE_PSRAM
```


## Library Build Options 

The library build options are defined as preprocessor macros (`#define name`).

Some options can be disabled to reduce program space.

### Predefined Options

The predefined options that are already set in [**FirebaseFS.h**](src/FirebaseFS.h) are following.

```cpp
ENABLE_NTP_TIME // For enabling the device or library time setup from NTP server
ENABLE_ERROR_STRING // For enabling the error string from error reason
FIREBASE_ENABLE_RTDB // For RTDB class compilation
FIREBASE_ENABLE_FIRESTORE // For Firestore compilation
FIREBASE_ENABLE_FCM // For Firebase Cloud Messaging compilation
FIREBASE_ENABLE_FB_STORAGE // For Firebase Storage compilation
FIREBASE_ENABLE_GC_STORAGE // For Google Cloud Storage compilation
FIREBASE_ENABLE_FB_FUNCTIONS // For Functions for Firebase compilation
FIREBASE_USE_PSRAM // For enabling PSRAM support
ENABLE_OTA_FIRMWARE_UPDATE // For enabling OTA updates support via RTDB, Firebase Storage and Google Cloud Storage buckets
USE_CONNECTION_KEEP_ALIVE_MODE // For enabling Keep Alive connection mode
DEFAULT_FLASH_FS // For enabling Flash filesystem support
DEFAULT_SD_FS // For enabling SD filesystem support 
CARD_TYPE_SD or CARD_TYPE_SD_MMC // The SD card type for SD filesystem
```

The Flash and SD filesystems are predefined.

SD is the default SD filesystem for all devices.

For ESP8266 and Arduino Pico, LittleFS is the default flash filesystem.

For ESP32 since v2.0.x, LittleFS is the default flash filesystem otherwise SPIFFS is the default flash filesystem.

In otherr devices, SPIFFS is the default flash filesystem.

User can change `DEFAULT_FLASH_FS` and `DEFAULT_SD_FS` with `CARD_TYPE_SD` or `CARD_TYPE_SD_MMC` defined values for other filesystems.

### Optional Options

The following options are not yet defined in [**FirebaseFS.h**](src/FirebaseFS.h) and can be assigned by user.

```cpp
FIREBASE_ETHERNET_MODULE_LIB `"EthernetLibrary.h"` // For the Ethernet library to work with external Ethernet module
FIREBASE_ETHERNET_MODULE_CLASS EthernetClass // For the Ethernet class object of Ethernet library to work with external Ethernet module
FIREBASE_ETHERNET_MODULE_TIMEOUT 2000 // For the time out in milliseconds to wait external Ethernet module to connect to network
ENABLE_ESP8266_ENC28J60_ETH //  For native core library ENC28J60 Ethernet module support in ESP8266
ENABLE_ESP8266_W5500_ETH // For native core library W5500 Ethernet module support in ESP8266
ENABLE_ESP8266_W5100_ETH // For native core library W5100 Ethernet module support in ESP8266
FIREBASE_DISABLE_ONBOARD_WIFI // For disabling on-board WiFI functionality in case external Client usage
FIREBASE_DISABLE_NATIVE_ETHERNET // For disabling native (sdk) Ethernet functionality in case external Client usage
FIREBASE_DEFAULT_DEBUG_PORT // For debug port assignment
```

You can assign the optional build options using one of the following methods.

- By creating user config file `CustomFirebaseFS.h` in library installed folder and define these optional options.

- By adding compiler build flags with `-D name`.

In PlatformIO IDE, using `build_flags` in PlatformIO IDE's platformio.ini is more convenient 

```ini
build_flags = -D DISABLE_FB_STORAGE
              -D EFIREBASE_DISABLE_ONBOARD_WIFI
```

For external Ethernet module integation used with function `setEthernetClient`, both `FIREBASE_ETHERNET_MODULE_LIB` and `FIREBASE_ETHERNET_MODULE_CLASS` should be defined.

`FIREBASE_ETHERNET_MODULE_LIB` is the Ethernet library name with extension (.h) and should be inside `""` or `<>` e.g. `"Ethernet.h"`.

`FIREBASE_ETHERNET_MODULE_CLASS` is the name of static object defined from class e.g. `Ethernet`.

`FIREBASE_ETHERNET_MODULE_TIMEOUT` is the time out in milliseconds to wait network connection.

For disabling predefined options instead of editing the [**FirebaseFS.h**](src/FirebaseFS.h) or using `#undef` in `CustomFirebaseFS.h`, you can define these build flags with these names or macros in `CustomFirebaseFS.h`.

```cpp
DISABLE_NTP_TIME // For disabling the NTP time setting
DISABLE_ERROR_STRING // For disabling the error string from error reason
DISABLE_RTDB // For disabling RTDB support
DISABLE_FIRESTORE // For disabling Firestore support
DISABLE_FCM // For disabling Firebase Cloud Messaging support
DISABLE_FB_STORAGE // For disabling Firebase Storage support
DISABLE_GC_STORAGE // For disabling Google Cloud Storage support
DISABLE_FB_FUNCTIONS // For disabling Functions for Firebase support
DISABLE_PSRAM // For disabling PSRAM support
DISABLE_OTA // For disabling OTA updates support
DISABLE_KEEP_ALIVE // For disabling TCP Keep Alive support (See TCP Keep Alive)
DISABLE_SD // For disabling flash filesystem support
DISABLE_FLASH // For disabling SD filesystem support
DISABLE_DEBUG // For disable debug port

FIREBASE_DISABLE_ALL_OPTIONS // For disabling all predefined build options above
```

Note that, `CustomFirebaseFS.h` for user config should be placed in the library install folder inside src folder.

This `CustomFirebaseFS.h` will not change or overwrite when update the library.



## TCP Client Options

### TCP Keep Alive


The TCP KeepAlive can be enabled from executing `<FirebaseData>.keepAlive` with providing TCP options as arguments, i.e.,

`tcpKeepIdleSeconds`, `tcpKeepIntervalSeconds` and `tcpKeepCount`.

Ex.

```cpp
fbdo.keepAlive(5 /* tcp KeepAlive idle 5 seconds */, 5 /* tcp KeeAalive interval 5 seconds */, 1 /* tcp KeepAlive count 1 */);

// If one of three arguments is zero, the KeepAlive will be disabled.
```

To check the KeepAlive status, use `<FirebaseData>.isKeepAlive`.


For the TCP (KeepAlive) options, see [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/lwip.html#tcp-options).

You can check the server connecting status, by executing `<FirebaseData>.httpConnected()` which will return true when connection to the server is still alive. 


The TCP KeepAlive was currently available in ESP32 unless in ESP8266, [this ESP8266 PR #8940](https://github.com/esp8266/Arduino/pull/8940) should be merged in the [ESP8266 Arduino Core SDK](https://github.com/esp8266/Arduino/releases), i.e., it will be supported in the ESP8266 core version newer than v3.1.2.


In ESP8266 core v3.1.2 and older, the error can be occurred when executing `<FirebaseData>.keepAlive` because of object slicing.


The Arduino Pico is currently not support TCP KeepAlive until it's implemented in WiFiClientSecure library as in ESP8266.

 
For External Client, this TCP KeepAlive option is not appliable and should be managed by external Client library.


### TCP Time Out

  The timeout for TCP client can be set from `FirebaseConfig` object included following.

  - Network reconnect timeout (interval) in ms (10 sec - 5 min) when network or WiFi disconnected.

    Ex. `config.timeout.networkReconnect = 10 * 1000;`

  - Socket connection and SSL handshake timeout in ms (1 sec - 1 min).

    Ex. `config.timeout.socketConnection = 10 * 1000;`

  - Server response read timeout in ms (1 sec - 1 min).

    Ex. `config.timeout.serverResponse = 10 * 1000;`

  - RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  
    Ex. `config.timeout.rtdbKeepAlive = 45 * 1000;`

  - RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.

    Ex. `config.timeout.rtdbStreamReconnect = 1 * 1000;`

  - RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the `readStream` will return false (error) when it called repeatedly in loop.

    Ex. `config.timeout.rtdbStreamError = 3 * 1000;`



## Firebase Authentications Options

The Firebase authentications methods support by this library included none or no-auth, legacy token (database secret) for RTDB, ID token, custom token and OAuth2.0 token.

See [other authentication examples](/examples/Authentications) for more authentications usage.

Some authentication methods require the token generaion and exchanging process which take more time than using the legacy token.

The system time must be set before authenticate using the custom and OAuth2.0 tokens or when the root certificate was set for verification. 

The authentication with custom and OAuth2.0 tokens takes the time, several seconds in overall process which included the NTP server time request, JWT token generation and signing process.

By setting the system (device) time prior to calling the **`Firebase.begin`**, the internal NTP server time request process will be ignored.

You can set the system time from the RTC chip or set manually by calling **`Firebase.setSystemTime`**.


While authenticating using Email and password, the process will be perform faster because no token generation and NTP server time request required. 

The authentication using the legacy token (database secret) does not have the delay because the token is always available for use.



### Access in Test Mode (No Auth)

In Test Mode, token generation will be ignored and no authentication applied to the request.

For RTDB, you can access RTDB database in Test Mode by set the security rules like this.

```json
{
  "rules": {
    ".read": true, 
    ".write": true
  }
}
```
And set the `config.signer.test_mode = true;`, see [TestMode.ino](/examples/Authentications/TestMode/TestMode.ino) example.

For Cloud Firestore and Firebase Storage, also set `config.signer.test_mode = true;` and modify the rules for the public access to test.


### The authenication credentials and prerequisites


To use Email/Password sign-in authentication as in the examples, the Email/Password Sign-in provider must be enabled by follow the following steps.



![Enable Email/Password Sign-in provider](/media/images/Enable_Email_Password_Provider.png)

![Enable Email/Password Sign-in provider](/media/images/Enable_Email_Password_Provider2.png)



Add Email and password for first user in your project then use this Email and password to sign in.

![Enable Email/Password Sign-in provider](/media/images/Enable_Email_Password_Provider3.png)



To use Anonymous sign-in, the Anonymous Sign-in provider must be enabled by follow the below steps.

![Enable Anonymous Sign-in provider](/media/images/Anonymous1.png)

![Enable Anonymous Sign-in provider](/media/images/Anonymous2.png)

![Enable Anonymous Sign-in provider](/media/images/Anonymous3.png)



To get API Key used in sign-in/sign-up authentication

![API Key](/media/images/API_Key.png)



To get the Service accounts key JSON file used in Custom and OAuth2.0 tokens athentications.

![Service Account Key File](/media/images/Service_Account_Key.png)



For RTDB usages, create new real-time database (if not setup yet)

![Firebase Host](/media/images/Create_New_RTDB.png)

![Firebase Host](/media/images/Create_New_RTDB2.png)

![Firebase Host](/media/images/Create_New_RTDB3.png)



Edit the default database rules as following



![Firebase Host](/media/images/Create_New_RTDB4.png)

```json
{
  "rules": {
    ".read": "auth != null", 
    ".write": "auth != null"
  }
}
```

[This document](https://firebase.google.com/docs/database/security) provides the RTDB security rules details.

The Firebase RTDB security rules are JSON-based rules which it should valid to used with this library RTDB functions that involved the security rules modification and reading, otherwise the rules wont be changed or read by these functions.


To get the database URL and secret (legacy token).

![Firebase Host](/media/images/RTDB_URL.png)

![Firebase Auth](/media/images/RTDB_Secret.png)



For server SSL authentication by providing the server root certificate.

Server SSL certificate verification is the process to ensure that the server that client is being connected is a trusted (valid) server instead of fake server.

The Google's GlobalSign R2 root certificate can be download from https://pki.goog/repository/

Select the .PEM (base-64 encoded string) or .DER (binary) file to download.

From the test as of July 2021, GlobalSign Root CA was missing from Google server, the certificate chain, GTS Root R1 can be used instead of root certificate.

![Firebase Host](/media/images/PEM_Download.png)

Below is how to assign the certificate data for server verification.

```cpp
  /* In case the certificate data was used  */
  config.cert.data = rootCACert;

  // Or custom set the root certificate for each FirebaseData object
  fbdo.setCert(rootCACert);

  /* Or assign the certificate file */

  /** From the test as of July 2021, GlobalSign Root CA was missing from Google server
   * as described above, GTS Root R1 (gsr1.pem or gsr1.der) can be used instead.
   * ESP32 Arduino SDK supports PEM format only even mBedTLS supports DER format too.
   * ESP8266 SDK supports both PEM and DER format certificates.
  */
  // config.cert.file = "/gsr1.pem";
  // config.cert.file_storage = mem_storage_type_flash; // or mem_storage_type_sd
```

## TCP Session Reusage and Asynchronous operation

The `FirbaseData` object is the class object that used as the user data container which used to pass to all library functions to keep the server response payload and TCP session.

Since version 5, you can call all functions without `FirbaseData` object with asynchronous operation supports. 

The `FirbaseData` object will be used in case where you want to re-use the TCP session to avoid to do SSL handshake everytime you want to sending request to server especially when `FirbaseData` object was defined in global or usage scope.


The transfer speed is faster than withou using `FirbaseData` object because of no additional delay for SSL handshake required in subsequence calls.

Without using `FirbaseData` object, the SSL handshake time was much reduced in the subsequence request because of SSL session resumption was implemented since version 5.

The asynchronous operation was done by assign the DataCallback function to `FirbaseData` object before use it or assign the assign the `DataCallback` to the supported functions directly in case no `FirbaseData` object was assigned. 


## Realtime Database

See [RTDB examples](/examples/RTDB) for complete usages.


### Read Data

Data at a specific node in Firebase RTDB can be read through the `get` function.

As noted in [Breaking Changes from version 4](#breaking-changes-from-version-4), the functions `getInt`, `getFloat`, `getDouble`, `getBool`, `getString`, `getJSON`, `getArray`, `getBlob`, `getFile` were removed and replaced by `get`.


The function `get` returns the boolean value indicates the success of the operation which will be `true` if all of the following conditions were met.

* Server returns HTTP status code 200

* The data types matched between request and response.

Since version 5, you can call `get` without `FirbaseData` object with asynchronous operation supports. 


Since version 5, the data or value to be taken from calling `get` can be obtained from the `FirbaseData` object that passed to the function, or `FirbaseData` object in the `DataCallback` function, or from the object or variable that passed to the function to store the value.


The type of received data can be obtained from `fbdo.dataType()` and `fbdo.dataTypeEnum()`.

The `fbdo.dataType()` returns `string`, `boolean`, `int`, `float`, `double`, `json`, `array`, `blob`, `file` or `null`.

The`fbdo.dataTypeEnum()` returns `firebase_rtdb_data_type_null` (1), `firebase_rtdb_data_type_integer`, `firebase_rtdb_data_type_float`, `firebase_rtdb_data_type_double`, `firebase_rtdb_data_type_boolean`, `firebase_rtdb_data_type_string`, `firebase_rtdb_data_type_json`, `firebase_rtdb_data_type_array`, `firebase_rtdb_data_type_blob`, and `firebase_rtdb_data_type_file` (10).

To get the value from `FirebaseData` object, use `fbdo.to<T>()` which `T` is the class or variable type name.

The following examples showed how to get value from `FirebaseData` object.

* `String s = fbdo.to<String>();`

* `std::string _s = fbdo.to<std::string>();`

* `const char *str = fbdo.to<const char *>();`

* `bool b = fbdo.to<bool>();`

* `int16_t _i = fbdo.to<int16_t>();`

* `int i = fbdo.to<int>();`

* `double d = fbdo.to<double>();`

* `float f = fbdo.to<float>();`

* `FirebaseJson *json = fbdo.to<FirebaseJson *>();` or

* `FirebaseJson &json = fbdo.to<FirebaseJson>();`

* `FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();` or

* `FirebaseJsonArray &arr = fbdo.to<FirebaseJsonArray>();`

* `std::vector<uint8_t> *blob = fbdo.to<std::vector<uint8_t> *>();`

* `File file = fbdo.to<File>();`


The BLOB and file stream data are stored as special base64 encoded string which are only supported and implemented in this library.

The encoded base64 string will be prefixed with some header string (`"file,base64,"` and `"blob,base64,"`) for data type manipulation. 


The following example showed how to get value from node "/test/data".


```cpp
  if (Firebase.RTDB.get(&fbdo, "/test/data")) {

    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer) {
      Serial.println(fbdo.to<int>());
    }

  } else {
    Serial.println(fbdo.errorReason());
  }
```

The asynchronous operation can be achieved by assigning the callback function to `FirebaseData` object.

As long as the function `Firebase.ready()` was executed in `loop()`, the callback function will be called when data is available or some events occurred.

The type of callback event can be checked from `callbackEventType` and the following five events can be happened. 

`firebase_callback_event_request_sent` occurred when device sending request.

`firebase_callback_event_response_timed_out` occurred when the response reading was timed out or RTDB stream was timed out.

`firebase_callback_event_response_error` occurred when the response reading error or RTDB stream error occurred.

`firebase_callback_event_response_received` occurred when server response was received.

`firebase_callback_event_response_keepalive` occurred when server sending keep-alive data in RTDB stream.

`firebase_callback_event_response_auth_revoked` occurred when authentication is revoked or auth token was expired in case RTDB stream.

Because data from the callback is asynchronously received, to identify which function or task that this data requested from, you can assign the request ID per task via `setRequestId`.

```cpp
// Assume 100 is the ID of this request.
fbdo.setRequestId(100);

Firebase.RTDB.getBool(&fbdo, "/test/bool");

```

In the callback function, check the ID when data received.

```cpp
void responseCallback(FirebaseData &data)
{
  if (data.getRequestId() == 100)
  {
     Serial.println(data.to<bool>());
  }
}
```

```cpp

fbdo.dataCallback = responseCallback;

// Assign NULL to fbdo.dataCallback to remove the callback.

if (!Firebase.RTDB.getInt(&fbdo, "/test/int"))
{
  Serial.println(fbdo.errorReason());
}

void responseCallback(FirebaseData &data)
{
  if (data.callbackEventType() == firebase_callback_event_request_sent)
    Serial.println("Request sent");
  else if (data.callbackEventType() == firebase_callback_event_response_timed_out)
    Serial.println("Response timed out");
  else if (data.callbackEventType() == firebase_callback_event_response_error)
    Serial.printf("Response error, %s", data.errorReason());
  else if (data.callbackEventType() == firebase_callback_event_response_received)
  {
    Serial.println("Response received");
    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
    {
      Serial.println(fbdo.to<int>());
    }
  }
}

```

### Store Data

To store data at a specific node in Firebase RTDB, use these set functions.

The function included `set`, `setInt`, `setFloat`, `setDouble`, `setBool`, `setString`, `setJSON`, `setArray`, `setBlob` and `setFile`. 

For generic set, use Firebase.RTDB.set(&fbdo, \<path\>, \<any variable or value\>).

As noted in [Breaking Changes from version 4](#breaking-changes-from-version-4), the `setAsync` abd `setXXXAsync` were removed. 

The functions return boolean value indicates the success of the operation which will be `true` if all of the following conditions matched.

* Server returns HTTP status code 200

* The data types matched between request and response.


Only setBlob and setFile functions that make a silent request to Firebase server, thus no payload response returned. 

The **priority**, virtual node **".priority"** of each database node can be set through Firebase's set functions.

The priority value can be used in a query or filtering the children's data under a defined node.

Priority option was removed from File and Blob functions since v2.4.0.

**ETag** (unique identifier value) assigned to Firebase's set functions is used as conditional checking.

If defined Etag is not matched the defined path's ETag, the set operation will fail with result **412 Precondition Failed**.

ETag at any node can be read through `Firebase.RTDB.getETag`.  ETag value changed upon the data was set or delete.

The server's **Timestamp** can be stored in the database through `Firebase.RTDB.setTimestamp`. 

The returned **Timestamp** value can get from `fbdo.to<int>()`. 

The following example showed how to store string to "/test/string".


```cpp

if (!Firebase.RTDB.setString(&fbdo, "/test/string", "hello"))
{
  Serial.println(fbdo.errorReason());
}
```


Similare to [Read Data](#read-data), if callback function was assigned to `FirebaseData` object, it will operate in async mode.

```cpp

fbdo.dataCallback = responseCallback;

// Assign NULL to fbdo.dataCallback to remove the callback.

if (!Firebase.RTDB.setString(&fbdo, "/test/string", "hello"))
{
  Serial.println(fbdo.errorReason());
}

void responseCallback(FirebaseData &data)
{
  if (data.callbackEventType() == firebase_callback_event_request_sent)
    Serial.println("Request sent");
  else if (data.callbackEventType() == firebase_callback_event_response_timed_out)
    Serial.println("Response timed out");
  else if (data.callbackEventType() == firebase_callback_event_response_error)
    Serial.printf("Response error, %s", data.errorReason());
  else if (data.callbackEventType() == firebase_callback_event_response_received)
  {
    Serial.println("Response received");
    Serial.println(fbdo.to<String>());
  }
}

```

Eventhough the functions `setAsync` and `setXXAsync` were removed, you can still ignore the server response and starting the next request immediately by calling `setWaitResponse(false)` from `FirebaseData` object and calling `waitStatus()` to check its waiting status.

The skipping server response via function `setWaitResponse` will be applied only when the callback was assign to `FirebaseData`. Without setting the callback, the server response skipping will not be taken place.

```cpp
// For simplicity of demonstration, we use the labda function as callback and use setWaitResponse inside it.
fbdo.dataCallback = [](FirebaseData &d){ d.setWaitResponse(false); };
```

Skipping the server response can cause unexpected result then using it is your own risk.


### Append Data

To append new data to a specific node in Firebase RTDB, use these push functions.

The function included `push`, `pushInt`, `pushFloat`, `pushDouble`, `pushBool`, `pushString`, `pushJSON`, `pushArray`, `pushBlob`, and `pushFile`.

These functions return boolean value indicates the success of the operation.

As noted in [Breaking Changes from version 4](#breaking-changes-from-version-4), the `pushAsync` abd `pushXXXAsync` were removed. 

The **unique key** of a new appended node can be determined from `fbdo.pushName()`.

As set functions, the Firebase's push functions support **priority**.

**ETag** was not available after push unless read the **ETag** at that new appended unique key later with `Firebase.RTDB.getETag`.

The server's **Timestamp** can be appended in the database through `Firebase.RTDB.pushTimestamp`.

The unique key of Timestamp can be determined after Timestamp was appended.


The following example showed how to append new data (using FirebaseJson object) to node "/test/append.


```cpp

FirebaseJson json;
FirebaseJson json2;

json2.add("child_of_002", 123.456);
json.add("parent_001", "parent 001 text");
json.add("parent 002", json2);

if (Firebase.RTDB.pushJSON(&fbdo, "/test/append", &json)) {

  Serial.println(fbdo.dataPath());
  Serial.println(fbdo.pushName());
  Serial.println(fbdo.dataPath() + "/"+ fbdo.pushName());

} else {
  Serial.println(fbdo.errorReason());
}
```

Similare to [Store Data](#store-data), if callback function was assigned to `FirebaseData` object, it will operate in async mode.


```cpp

fbdo.dataCallback = responseCallback;

FirebaseJson json;
FirebaseJson json2;

json2.add("child_of_002", 123.456);
json.add("parent_001", "parent 001 text");
json.add("parent 002", json2);

// Assign NULL to fbdo.dataCallback to remove the callback.

if (!Firebase.RTDB.pushJSON(&fbdo, "/test/append", &json))
{
  Serial.println(fbdo.errorReason());
}

void responseCallback(FirebaseData &data)
{
  if (data.callbackEventType() == firebase_callback_event_request_sent)
    Serial.println("Request sent");
  else if (data.callbackEventType() == firebase_callback_event_response_timed_out)
    Serial.println("Response timed out");
  else if (data.callbackEventType() == firebase_callback_event_response_error)
    Serial.printf("Response error, %s", data.errorReason());
  else if (data.callbackEventType() == firebase_callback_event_response_received)
  {
    Serial.println("Response received");
    Serial.println(fbdo.dataPath());
    Serial.println(fbdo.pushName());
    Serial.println(fbdo.dataPath() + "/"+ fbdo.pushName());
  }
}

```

Eventhough the functions `pushAsync` and `pushXXAsync` were removed, you can still ignore the server response and starting the next request immediately by calling `setWaitResponse(false)` from `FirebaseData` object and calling `waitStatus()` to check its waiting status.

The skipping server response via function `setWaitResponse` will be applied only when the callback was assign to `FirebaseData`. Without setting the callback, the server response skipping will not be taken place.

```cpp
// For simplicity of demonstration, we use the labda function as callback and use setWaitResponse inside it.
fbdo.dataCallback = [](FirebaseData &d){ d.setWaitResponse(false); };
```

Skipping the server response can cause unexpected result then using it is your own risk.


### Patch Data

Firebase's update functions used to patch or update new or existing data at the defined node.

As noted in [Breaking Changes from version 4](#breaking-changes-from-version-4), the `updateNode`, `updateNodeAsync`, `updateNodeSilent` and `updateNodeSilentAsync` were removed and replaced by `update`. 

The functions, `update` is available and work with JSON object (`FirebaseJson` object only).

If any key name provided at a defined node in JSON object has not existed, a new key will be created.

The server returns JSON data payload which was successfully patched.

By assigning `dataCallback` to `FirebaseData` object, the payload will be return otherwise no payload returned.

Return of large JSON payload will cost the network bandwidth, assigning `NULL` as `dataCallback` to `FirebaseData` will save the network bandwidth.


The following example showed how to patch data at "/test".


```cpp

FirebaseJson updateData;
FirebaseJson json;
json.add("_data2","_value2");
updateData.add("data1","value1");
updateData.add("data2", json);

if (Firebase.RTDB.update(&fbdo, "/test/update", &updateData)) {

  Serial.println(fbdo.dataPath());
  Serial.println(fbdo.dataType());
  Serial.println(fbdo.to<String>()); 

} else {
  Serial.println(fbdo.errorReason());
}
```


Similare to [Store Data](#store-data), if callback function was assigned to `FirebaseData` object, it will operate in async mode.

```cpp

fbdo.dataCallback = responseCallback;

// Assign NULL to fbdo.dataCallback to remove the callback.

if (!Firebase.RTDB.update(&fbdo, "/test/int", 123))
{
  Serial.println(fbdo.errorReason());
}

void responseCallback(FirebaseData &data)
{
  if (data.callbackEventType() == firebase_callback_event_request_sent)
    Serial.println("Request sent");
  else if (data.callbackEventType() == firebase_callback_event_response_timed_out)
    Serial.println("Response timed out");
  else if (data.callbackEventType() == firebase_callback_event_response_error)
    Serial.printf("Response error, %s", data.errorReason());
  else if (data.callbackEventType() == firebase_callback_event_response_received)
  {
    Serial.println("Response received");
    Serial.println(fbdo.dataPath());
    Serial.println(fbdo.dataType());
    Serial.println(fbdo.to<String>()); 
  }
}

```

Eventhough the functions `updateNodeAsync` and `updateNodeSilentAsync` were removed, you can still ignore the server response and starting the next request immediately by calling `setWaitResponse(false)` from `FirebaseData` object and calling `waitStatus()` to check its waiting status.

The skipping server response via function `setWaitResponse` will be applied only when the callback was assign to `FirebaseData`. Without setting the callback, the server response skipping will not be taken place.

```cpp
// For simplicity of demonstration, we use the labda function as callback and use setWaitResponse inside it.
fbdo.dataCallback = [](FirebaseData &d){ d.setWaitResponse(false); };
```

Skipping the server response can cause unexpected result then using it is your own risk.


### Remove Data

As noted in [Breaking Changes from version 4](#breaking-changes-from-version-4), the `deleteNode` was removed and replaced by `remove`. 

The following example showed how to remove data and its children at node "/test/append"

```cpp
Firebase.RTDB.remove(&fbdo, "/test/append");
```



### Filtering Data

To filter or query the data, the following query parameters are available through the QueryFilter class.

These parameters are `orderBy`, `limitToFirst`, `limitToLast`, `startAt`, `endAt`, and `equalTo`.

To filter data, parameter `orderBy` should be assigned.

Use **"$key"** as the `orderBy` parameter if the key of child nodes was used for the query.

Use **"$value"** as the `orderBy` parameter if the value of child nodes was used for the query.

Use **key (or full path) of child nodes** as the `orderBy` parameter if all values of the specific key were used for the query.

Use **"$priority"** as `orderBy` parameter if child nodes's **"priority"** was used for query.



The above `orderBy` parameter can be combined with the following parameters for limited and ranged the queries.

`QueryFilter.limitToFirst` -  The total children (number) to filter from the first child.

`QueryFilter.limitToLast` -   The total last children (number) to filter. 

`QueryFilter.startAt` -       Starting value of range (number or string) of query upon orderBy param.

`QueryFilter.endAt` -         Ending value of range (number or string) of query upon orderBy param.

`QueryFilter.equalTo` -       Value (number or string) matches the orderBy param



The following example showed how to use queries parameter in QueryFilter class to filter the data at node "/test/data"

```cpp
// Assume that children that have key "sensor" are under "/test/data"

// Instantiate the QueryFilter class
QueryFilter query;

// Build query using specified child node key "sensor" under "/test/data"
query.orderBy("sensor");

// Query any child that its value begins with 2 (number), assumed that its data type is float or integer
query.startAt(2);

// Query any child that its value ends with 8 (number), assumed that its data type is float or integer
query.endAt(8);

// Limit the maximum query result to return only the last 5 nodes
query.limitToLast(5);


if (Firebase.RTDB.getJSON(&fbdo, "/test/data", &query))
{
  // Success, then try to read the JSON payload value
  Serial.println(fbdo.to<String>());
}
else
{
  // Failed to get JSON data at defined node, print out the error reason
  Serial.println(fbdo.errorReason());
}

// Clear all query parameters
query.clear();
```


### Monitoring Data

This library uses HTTP GET request with `text/event-stream` header to make [**HTTP streaming**](https://en.wikipedia.org/wiki/Server-sent_events) connection.

As noted in [Breaking Changes from version 4](#breaking-changes-from-version-4), the `beginStream`, `beginStream`, `endStream`, `removeStreamCallback`, `beginMultiPathStream`, `setMultiPathStreamCallback`, and `removeMultiPathStreamCallback` were removed and replaced with `stream`, `setDataCallback`, `readStream` and `stopStream`.

There are three modes to operate stream in this library i.e. with using `FirebaseData` (manually and using callback function) and without using `FirebaseData`.

#### Operate stream manualy with `FirebaseData` object

Calling the function `stream` by passing the pointer to globally defined `FirebaseData` object and path as parameters.

Polling get stream's event data by calling `readStream` in `loop()`.

Checking the available stream event data from `FirebaseData` that passed to `stream` by calling its member function `streamAvailable` then taking the data from it if event data is available.

Note that, when using the shared `FirebaseData` object for stream and CRUD usages(normal operation to create,read, update and remove data), the stream connection will be interrupted (closed) to connect in other HTTP mode, the stream will be resumed (open) after the CRUD usages.

For the above case, you need to provide the idle time for `FirebaseData` object to establish the streaming connection and get the stream payload. The changes on the server at the streaming node path during the stream interruption will be missed.

To avoid this sitation, don't share the usage of stream's `FirebaseData` object, another `FirebaseData` object should be used.

In addition, delay function used in the same loop of `readStream()` will defer the streaming, the server data changes may be missed.


#### Operate stream with Callback and `FirebaseData` object

Calling the function `stream` by passing the pointer to globally defined `FirebaseData` object, path and `DataCallback` as parameters.

The callback function `DataCallback` is the function that `FirebaseData` was pass by reference to the function.

Take and process event data from the `FirebaseData` that passed in to the callback function.

In ESP32, the FreeRTOS task will be created when calling `stream` to operate stream repeatedly.

In ESP8266, the new schedule task will be set to run repeatedly.

For other devices (not ESP32 and ESP8266), you need to call `runStream` in `loop()` to polling get stream's event data.

The stack memory for ESP32 stream FreeRTOS task is 8192 which can be assign via build flag or maro `FIREBASE_RTDB_STREAM_TASK_STACK_SIZE` in `FirebaseFS.h` or `CustomFirebaseFS.h`.

By assihning `NULL` as `DataCallback` or later set by calling `FirebaseData` object member's function`setDataCallback`, the stream taks will stop immediately.

Note that, when `runStream` was executed in `loop()`, the infinite loop tasks (ESP32 FreeRTOS task and ESP8266 Schedule task) will stop and the stream will be operate via `runStream` only.


#### Operate stream with Callback (no `FirebaseData` object)


Calling the function `stream` by passing the path, `DataCallback` and `slot` number as parameters.

This operating mode is similar to [Operate stream with Callback and `FirebaseData` object](#operate-stream-with-callback-and-firebasedata-object) unless no `FirebaseData` object required.

The internal `FirebaseData` object will be created internally and its ID was assigned via `slot` number.

The maximum numbers of `slot` is 10 or can be assign via build flag or maro `FIREBASE_RTDB_STREAM_MAX_INTERNAL_SLOT` in `FirebaseFS.h` or `CustomFirebaseFS.h`.

For non-ESP devices, the function `runStream` should be executed in `loop()`.

There is some limitation for this stream operating mode, the default `BearSSL` RX and TX buffer sizes are 2048 and 512 which the large streaming data may not be received and causes the stream timed out. 

For large stream event data, use global defined`FirebaseData` object for stream instead.


#### Stop stream operation

To stop stream operation that is currently running, use `stopStream` with the pointer to `FirebaseData` object that you want to stop as parameter.


In case of no `FirebaseData` object stream, to stop internal stream, the `slot` number should be pass to function `stopStream`.

To stop all stream operations, call `stopStream()` (without parameter).


To pause stream operation that using `FirebaseData` object, call `pauseFirebase(true)` from the `FirebaseData` object.



### Enable TCP KeepAlive for reliable HTTP Streaming

In general, the RTDB stream timed out occurred when no data included keep-alive event data received in the specific period (45 seconds) which can be set via `config.timeout.rtdbKeepAlive`.

Now you can take the pros of TCP KeepAlive in Stream mode by probing the server connection at some intervals to help the stream time out more reliable.

You can check the server connecting status, by executing `<FirebaseData>.httpConnected()` which will return true when connection to the server is still alive. 

As previousely described, using [TCP KeepAlive in `FirebaseData` object](#about-firebasedata-object) in Stream has pros and cons.

The TCP KeepAlive can be enabled from executing `<FirebaseData>.keepAlive` with providing TCP options as arguments, i.e.,

`tcpKeepIdleSeconds`, `tcpKeepIntervalSeconds` and `tcpKeepCount`.

Ex.

```cpp
stream.keepAlive(5 /* tcp KeepAlive idle 5 seconds */, 5 /* tcp KeeAalive interval 5 seconds */, 1 /* tcp KeepAlive count 1 */);
```


### HTTP Streaming examples

Callback with FirebaseData object

```cpp

// In setup(), set the streaming path to "/test/data" and begin stream connection.

if (!Firebase.RTDB.Stream(&fbdo, "/test/data", dataCallback))
{
  // Could not begin stream connection, then print out the error detail.
  Serial.println(fbdo.errorReason());
}

  
  // Global function that handles stream data
void dataCallback(FirebaseData &data)
{

  if (data.isStream())
  {

    if (data.callbackEventType() == firebase_callback_event_response_keepalive)
    {
      Serial.println("Stream keep alive event");
    }
    else if (data.callbackEventType() == firebase_callback_event_response_auth_revoked)
    {
      Serial.println("Stream auth revoked event");
    }
    else if (data.callbackEventType() == firebase_callback_event_response_timed_out)
    {
      Serial.println("Stream timed out");

      if (!stream.httpConnected())
        Serial_Printf("error code: %d, reason: %s\n\n", data.httpCode(), data.errorReason().c_str());
    }
    else if (data.callbackEventType() == firebase_callback_event_response_error)
       Serial.printf("Stream error, %s", data.errorReason());
    else if (data.callbackEventType() == firebase_callback_event_response_received)
    {
      Serial.println("Stream payload received");

      Serial_Printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                    data.streamPath().c_str(),
                    data.dataPath().c_str(),
                    data.dataType().c_str(),
                    data.eventType().c_str());

      if (data.dataTypeEnum() == firebase_rtdb_data_type_integer)
        Serial.println(data.to<int>());
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_float)
        Serial.println(data.to<float>(), 5);
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_double)
        printf("%.9lf\n", data.to<double>());
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_boolean)
        Serial.println(data.to<bool>() ? "true" : "false");
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_string)
        Serial.println(data.to<String>());
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_json)
      {
        FirebaseJson *json = data.to<FirebaseJson *>();
        Serial.println(json->raw());
      }
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_array)
      {
        FirebaseJsonArray *arr = data.to<FirebaseJsonArray *>();
        Serial.println(arr->raw());
      }

      Serial.println();
    }
  }

}

// For authentication except for legacy token, Firebase.ready() should be called repeatedly 
// in loop() to handle authentication tasks.

void loop()
{
  if (Firebase.ready())
  {
    // Firebase is ready to use now.

  }
}

```

The following example showed how to subscribe to the data changes at "/test/data" and polling the stream manually.

```cpp
// In setup(), set the streaming path to "/test/data" and begin stream connection.
if (!Firebase.RTDB.beginStream(&fbdo, "/test/data"))
{
  Serial.println(fbdo.errorReason());
}

// Place this in loop()
if (!Firebase.RTDB.readStream(&fbdo))
{
  Serial.println(fbdo.errorReason());
}

if (fbdo.streamTimeout())
{
  Serial.println("Stream timeout, resume streaming...");
  Serial.println();
}

if (fbdo.streamAvailable())
{
  if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
    Serial.println(fbdo.to<int>());
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_float)
    Serial.println(fbdo.to<float>(), 5);
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_double)
    printf("%.9lf\n", fbdo.to<double>());
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_boolean)
    Serial.println(fbdo.to<bool>() ? "true" : "false");
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string)
    Serial.println(fbdo.to<String>());
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_json)
  {
      FirebaseJson *json = fbdo.to<FirebaseJson *>();
      Serial.println(json->raw());
  }
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_array)
  {
      FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();
      Serial.println(arr->raw());
  }
}

// For authentication except for legacy token, Firebase.ready() should be called repeatedly 
// in loop() to handle authentication tasks.

void loop()
{
  if (Firebase.ready())
  {
    // Firebase is ready to use now.
    
  }
}

```



## Firebase Cloud Messaging (FCM)

The library acts as a app server to sends the message to registeration devices by sending request to the Google's FCM backend via the legacy HTTP and HTTPv1 APIs.

The functions available are setServerKey, send, subscibeTopic, unsubscibeTopic, appInstanceInfo and regisAPNsTokens.

Function `Firebase.FCM.setServerKey` to setup the Server Key which required by the legacy protocols.

Function `Firebase.FCM.send` to send the message with the selectable legacy and HTTPv1 messages constructors.  

Function `Firebase.FCM.subscribeTopic` to add the subscription for instance ID (IID) tokens to the defined topic.

Function `Firebase.FCM.unsubscribeTopic` to remove the subscription for instance ID (IID) tokens from the defined topic.

Function `Firebase.FCM.appInstanceInfo` to get the app instance info for a device. This also provides the subscribed topics info.

Function `Firebase.FCM.regisAPNsTokens` to create the registration tokens for iOS APNs tokens.

The library provides two message constructors that hold the data to construct the JSON object payload internally.

For legacy message, see https://firebase.google.com/docs/cloud-messaging/http-server-ref

For HTTPv1 message, see ttps://firebase.google.com/docs/reference/fcm/rest/v1/projects.messages

The HTTPv1 APIs requires OAUth2.0 authentication using the Service Account credential.



The following example showed how to send FCM message.

```cpp
// Provide your Firebase project's server key to send messsage using the legacy protocols
Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);

// Construct the legacy message
FCM_HTTPv1_JSON_Message msg;

// Assign the device registration token
msg.token = DEVICE_REGISTRATION_ID_TOKEN;

// Assign the notification payload
msg.notification.body = "Notification body";
msg.notification.title = "Notification title";

FirebaseJson json;
String payload;

// Assign the data payload
// all data key-values should be in string
json.add("humidity", "70");
json.toString(payload);
msg.data = payload.c_str();

// Send message
if (Firebase.FCM.send(&fbdo, &msg))
{
   erial.println("Message sent to FCM backend.");
   Serial.println(Firebase.FCM.payload(&fbdo));
}
else
{
   Serial.println("Something wrong, can't send request to FCM backend.");
   Serial.println(fbdo.errorReason());
}

```



## Firebase Cloud Firestore

This library implements a REST Client for Cloud Firestore database. The RPC APIs are not implemented in this library. 

The support functions for Cloud Firestore are export, import, create, patch, get, delete the document and list the documents and collection.

See the [Firestore examples](/examples/Firestore) for the usages.

If you see that some useful applications are missed from this library, you can contribute the code examples that can be usefull for other users.

The Cloud Firestore REST APIs are different than the APIs that used in Firebase SDK clients.

If you don't see the examples that can be applied for your works or you don't know how to use Firestore functions in this library, for better understanding the Cloud Firestore RESt APIs please see the [REST API reference document](https://firebase.google.com/docs/firestore/reference/rest/v1beta2/projects.databases/importDocuments) which you can test with its API Explorer. 

For Cloud Firestore REST API features which are not available in this API, you can request the features [here](https://firebase.google.com/support/troubleshooter/report/features). 


The unsecured security rules that allows the public usage of Firestore is

```
service cloud.firestore {
  match /databases/{database}/documents {
    match /todos/{document=**} {
      allow read, write: if true;
    }
  }
}
```

For secured Firestore usages, the security rules should include the auth field similar to this.

```
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    match /{document=**} {
      allow read, write: if request.auth.uid != null;
    }
  }
}
```
This [Google document](https://firebase.google.com/docs/firestore/security/get-started) provides the Firestore security rules details.

## Firebase Storage

The Firebase Storage bucket file upload, download, read its meta data and listing are supported. 

The [OTA firmware update](/examples/Storage/FirebaseStorage/DownloadFileOTA/DownloadFileOTA.ino) via the Strage bucket is supported.

See the [Firebase Storage examples](/examples/Storage/FirebaseStorage) for the usages.



## Google Cloud Storage

The Firebase or Google Cloud Storage bucket file upload, download, read its meta data and listing are supported.

This Google Cloud Storage allows large files upload which upload the large file via the Firebase Storage functions is not allowable.

The [OTA firmware update](/examples/Storage/GoogleCloudStorage/DownloadFileOTA/DownloadFileOTA.ino) via the Google Strage bucket is supported.


See the [Google Cloud Storage examples](/examples/Storage/GoogleCloudStorage) for the usages.



## Cloud Functions for Firebase

The Cloud Functions create/deployment, update, call, delete and list are supported.

The library also support the source code deployment which you can edit the functions and deploy it using the embedded device.

See [Cloud Functions examples](/examples/CloudFunctions) for complete usages.



### IAM Permission and API Enable


Some Firestore functions and all Cloud Functions functions requires the OAuth2.0 authentication and not allow the unauthentication and Email/password or custom token authenication access.

You may still get the error permission denined error even using OAuth2.0 authen with Service Account credentials, because the client in the Service Account does not have the Owner and Editor permissions.

To assign the Owner and Editor permissions to the client.

Go to https://console.cloud.google.com/iam-admin

Choose the project, look at the member which matches the client email in service account credentials. Edit the permission, add the role Owner under the Basic

![IAM Edit Permission 1](/media/images/IAM_Permission_Role1.png)

![IAM Edit Permission 2](/media/images/IAM_Permission_Role2.png)

![IAM Edit Permission 3](/media/images/IAM_Permission_Role3.png)

![IAM Edit Permission 4](/media/images/IAM_Permission_Role4.png)


Wait a few minutes for the action to propagate after adding roles. 


For Cloud Functions Cloud Build API must be enabled for the project. To enable Cloud Build API go to https://console.developers.google.com/apis/library/cloudbuild.googleapis.com



## Create, Edit, Serializing and Deserializing the JSON Objects


This library has built-in FirebaseJson Arduino library, the easiest JSON parser, builder and editor.

FirebaseJson usages are so simple as you read, store and update(edit) the JSON node in Firebase RTDB.

It doesn't use the recursive call to parse or deserialize complex or nested JSON objects and arrays. 

This makes the library can use with a limited memory device. 


Since you declare the FirebaseJson or FirebaseJsonArray object, use the functions `setJsonData`, `setJsonArrayData`, `add`, `set` and `remove` to build or edit the JSON/Array object and use `get` to parse the node's contents. 

Defined the relative path of the specific node to `add`, `set`, `remove` and `get` functions to add, set, remove and get its contents.


Function `FirebaseJson.setJsonData` is to deserialize the JSON string to JSON object.

In addition, function `FirebaseJson.readFrom` can be used to read the streaming JSON contents from WiFi/Ethernet Client, File and Harware Serial and serialize it as the streaming content contains valid JSON data. 


Function `FirebaseJson.add` is used to add the new node with the contents e.g. String, Number (int and double), Boolean, Array and Object to the defined node.


Function `FirebaseJson.set` is used for edit, overwrite, create new (if not exist) node with contents e.g. String, Number (int and double), Boolean, Array and Object at the defined relative path and node.


Function `FirebaseJson.get` is used for parsing or deserializee the JSON object and array. The deserialized or parsed result will keep in FirebaseJsonData object which can be casted to any type of value or variable e.g string, bool, int, float, double by using `FirebaseJsonData.to<type>`. 

The casting from FirebaseJsonData to FirebaseJson and FirebaseJsonArray objects is different, by using `FirebaseJsonData.getJSON(FirebaseJson)` and `FirebaseJsonData.getArray(FirebaseJsonArray)`.


Function `FirebaseJson.remove` is used to remove the node and all its children's contents at the defined relative path and node. 


Function `FirebaseJson.toString` is used for serializeing the JSON object to writable objects e.g. char array, Arduino String, C/C++ string, WiFi/Ethernet Client and Hardware/Software Serial.


Function `FirebaseJson.serializedBufferLength` is used for calculating the serialized buffer size that required for reserved buffer in serialization.


Function `FirebaseJson.responseCode` is used to get the http code response header while read the WiFi/Ethernet Client using `FirebaseJson.toString`.


Functions `FirebaseJson.iteratorBegin`, `FirebaseJson.iteratorGet` and `FirebaseJson.iteratorEnd` are used to parse all JSON object contents as a list which can be iterated with index.


Function `FirebaseJson.clear` is used to clear JSON object contents.


Function `FirebaseJson.setFloatDigits` is for float number precision when serialized to string.


Function `FirebaseJson.setDoubleDigits` is for double number precision when serialized to string.


Function `FirebaseJsonArray.add` is used for adding the new contents e.g. String, Number (int and double), Boolean, Array and Object to JSON array.


Function `FirebaseJsonArray.set` is for edit, overwrite, create new (if not exist) contents e.g. String, Number (int and double), Boolean, Array and Object at the defined relative path or defined index of JSON array.


Function `FirebaseJsonArray.get` and `FirebaseJsonArray.search`work in the same way as FirebaseJson objects


Function `FirebaseJsonArray.remove` is used to remove the array's contents at the defined relative path or defined index of JSON array.


Function `FirebaseJsonArray.toString` is used for serializeing the JSON array object to writable objects e.g. char array, Arduino String, C/C++ string, WiFi/Ethernet Client and Hardware/Software Serial.


Function `FirebaseJsonArray.serializedBufferLength` is used for calculating the serialized buffer size that required for reserved buffer in serialization.


Function `FirebaseJsonArray.responseCode` is used to get the http code response header while read the WiFi/Ethernet Client using `FirebaseJson.toString`.


Function `FirebaseJsonArray.clear` is used to clear JSON array object contents.


Function `FirebaseJsonArray.setFloatDigits` is for float number precision when serialized to string.


Function `FirebaseJsonArray.setDoubleDigits` is for double number precision when serialized to string.

See [examples/FirebaseJson](examples/FirebaseJson) for the usage.

For FirebaseJson function description, see [FirebaseJSON object Functions](src#firebasejson-object-functions).


The following example shows how to use FirebaseJson.

```cpp
// Declare FirebaseJson object (global or local)
FirebaseJson json;

// Add name with value Living Room to JSON object
json.add("name", "Living Room");

// Add temp1 with value 120 and temp1 with 40 to JSON object
// Note: temp2 is not the child of temp1 as in previous version.
json.add("temp1", 120).add("temp2", 40);

// Add nested child contents directly
json.set("unit/temp1", "Farenheit");
json.set("unit/temp2", "Celcius");

// Deserialize to serial with prettify option
json.toString(Serial, true);
Serial.println();
Serial.println();

/**
This is the result of the above code

{
    "name": "Living Room",
    "temp1": 120,
    "temp2": 40,
    "unit": {
        "temp1": "Farenheit",
        "temp2": "Celcius"
    }
}
*/

// To set array to the above JSON using FirebaseJson directly
// Set (add) array indexes 0,1,2,5,7 under temp1, the original value will be replaced with new one.
json.set("temp1/[0]", 47);
json.set("temp1/[1]", 28);
json.set("temp1/[2]", 34);
json.set("temp1/[5]", 23); // null will be created at array index 3,4 due to it's not yet assigned
json.set("temp1/[7]", 25); // null will be created at array index 6

// Print out as prettify string
json.toString(Serial, true);
Serial.println();
Serial.println();

/**
The result of the above code

{
    "name": "Living Room",
    "temp1": [
        47,
        28,
        34,
        null,
        null,
         23,
        null,
        25
     ],
    "temp2": 40,
    "unit": {
        "temp1": "Farenheit",
        "temp2": "Celcius"
    }
 }
*/

// Try to remove temp1 array at index 1
json.remove("temp1/[1]");

// Try to remove temp2
json.remove("temp2");

// Print out as prettify string
json.toString(Serial, true);
Serial.println();
Serial.println();

/**
The result of the above code

{
    "name": "Living Room",
    "temp1": [
         47,
         34,
         null,
         null,
         23,
         null,
         25
    ],
    "unit": {
        "temp1": "Farenheit",
        "temp2": "Celcius"
    }
}
*/

// Now parse/read the contents from specific node unit/temp2
// FirebaseJsonData is required to keep the parse results which can be accessed later
FirebaseJsonData result;

json.get(result, "unit/temp2");

if (result.success)
{
  // Print type of parsed data e.g string, int, double, bool, object, array, null and undefined
  Serial.println(result.type);
  // Print its content e.g.string, int, double, bool whereas object, array and null also can access as string
  Serial.println(result.to<String>());
  // Serial.println(result.to<int>());
  // Serial.println(result.to<bool>());
  // Serial.println(result.to<float>());
  // Serial.println(result.to<double>());
}

// The above code will show
/**
string
Celcius
*/

// To get the array temp from FirebaseJson

json.get(result, "temp1");

// Prepare FirebaseJsonArray to take the array from FirebaseJson
FirebaseJsonArray arr;

// Get array data
result.get<FirebaseJsonArray>(arr);

// Call get with FirebaseJsonData to parse the array at defined index i
for (size_t i = 0; i < arr.size(); i++)
{
  // result now used as temporary object to get the parse results
  arr.get(result, i);

  // Print its value
  Serial.print("Array index: ");
  Serial.print(i);
  Serial.print(", type: ");
  Serial.print(result.type);
  Serial.print(", value: ");
  Serial.println(result.to<String>());
}

/**
The result of above code
Array index: 0, type: int, value: 47
Array index: 1, type: int, value: 34
Array index: 2, type: null, value: null
Array index: 3, type: null, value: null
Array index: 4, type: int, value: 23
Array index: 5, type: null, value: null
Array index: 6, type: int, value: 25
*/
 

```



The following example shows how to use FirebaseJsonArray.

```cpp
// Declare FirebaseJsonArray object (global or local)
FirebaseJsonArray arr;

// Add some data
arr.add("banana");
arr.add("mango");
arr.add("coconut");


// Change the array contents
arr.set("[1]/food", "salad");
arr.set("[1]/sweet", "cake");
arr.set("[1]/appetizer", "snack");
arr.set("[2]", "apple"); // or arr.set(2, "apple");
arr.set("[4]/[0]/[1]/amount", 20);

// Print out array as prettify string
arr.toString(Serial, true);
Serial.println();
Serial.println();

/**
This is the result of the above code

[
    "banana",
    {
        "food": "salad",
        "sweet": "cake",
        "appetizer": "snack"
    },
    "apple",
    null,
    [
        [
            null,
            {
                "amount": 20
            }
        ]
    ]
]
*/

// Remove array content at /4/0/1/amount
arr.remove("[4]/[0]/[1]/amount");

// Print out as prettify string
arr.toString(Serial, true);
Serial.println();
Serial.println();
/**
The result of the above code

[
    "banana",
    {
        "food": "salad",
        "sweet": "cake",
        "appetizer": "snack"
    },
    "apple",
    null,
    [
        [
            null
        ]
    ]
]

*/

// Now parse/read the array contents at some index

FirebaseJsonData result;

arr.get(result, "[1]/food");

if(result.success)
{
  // Type of parsed data
  Serial.println(result.type);
  // Its value
  Serial.println(result.to<String>());
  // Serial.println(result.to<int>());
  // Serial.println(result.to<bool>());
  // Serial.println(result.to<float>());
  // Serial.println(result.to<double>());

}

// The above code will show
/**
string
salad
*/


// To get the JSON object at array index 1 from FirebaseJsonArray
arr.get(result, "[1]");// or arr.get(result, 1);

// Prepare FirebaseJson to take the JSON object from FirebaseJsonArray
FirebaseJson json;

// Get FirebaseJson data
result.get<FirebaseJson>(json);

// Parse the JSON object as list
// Get the number of items
size_t len = json.iteratorBegin();
FirebaseJson::IteratorValue value;
for (size_t i = 0; i < len; i++)
{
    value = json.valueAt(i);
    Serial.printf("%d, Type: %s, Name: %s, Value: %s\n", i, value.type == FirebaseJson::JSON_OBJECT ? "object" : "array", value.key.c_str(), value.value.c_str());
}

// Clear all list to free memory
json.iteratorEnd();


/**
The result of the above code

0, Type: object, Key: food, Value: salad
1, Type: object, Key: sweet, Value: cake
2, Type: object, Key: appetizer, Value: snack

*/


```

## Acheivement

### Open Sourcs Contribution Awards

This project **Firebase Arduino Client Library for ESP8266 and ESP32** wins the [Google Open Source Peer Bonus program](https://opensource.google/documentation/reference/growing/peer-bonus).

This project would not have been possible without support from all users.

Thanks for all contributions and Google Open Source.


## License

The MIT License (MIT)

Copyright (c) 2023 K. Suwatchai (Mobizt)


Permission is hereby granted, free of charge, to any person returning a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
