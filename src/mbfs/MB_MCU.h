#pragma once

#ifndef MB_MCU_H
#define MB_MCU_H


#if defined(ESP8266) || defined(ESP32)
#ifndef MB_ARDUINO_ESP
#define MB_ARDUINO_ESP
#endif
#endif

#if defined(__arm__)
#ifndef MB_ARDUINO_ARM
#define MB_ARDUINO_ARM 
#endif
#endif

#if defined(ARDUINO_ARCH_SAMD)
#ifndef MB_ARDUINO_ARCH_SAMD
#define MB_ARDUINO_ARCH_SAMD
#endif
#endif

#if defined(ARDUINO_ARCH_RP2040)

#if defined(ARDUINO_NANO_RP2040_CONNECT)
#ifndef MB_ARDUINO_NANO_RP2040_CONNECT
#define MB_ARDUINO_NANO_RP2040_CONNECT
#endif
#else
#ifndef MB_ARDUINO_PICO
#define MB_ARDUINO_PICO
#endif
#endif

#endif


#if defined(TEENSYDUINO)
#ifndef MB_ARDUINO_TEENSY
#define MB_ARDUINO_TEENSY
#endif
#endif


#endif