#ifndef ESP_SUSPEND_H__
#define ESP_SUSPEND_H__
#include <Arduino.h>

#if defined(ESP8266)
#if __has_include(<core_esp8266_version.h>)
#include <core_esp8266_version.h>
#endif
#endif

__attribute__((unused)) static void esp_bssl_idle()
{
#if defined(ARDUINO_ESP8266_MAJOR) && defined(ARDUINO_ESP8266_MINOR) && defined(ARDUINO_ESP8266_REVISION) && ((ARDUINO_ESP8266_MAJOR == 3 && ARDUINO_ESP8266_MINOR >= 1) || ARDUINO_ESP8266_MAJOR > 3)
  esp_yield();
#else
  delay(0);
#endif
}

#endif