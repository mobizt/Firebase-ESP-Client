
#include <Arduino.h>
#include "./mbfs/MB_MCU.h"
#include "./FirebaseFS.h"

#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)

#ifndef FIREBASE_LFCM_H
#define FIREBASE_LFCM_H
#include "./FB_Utils.h"

class FCMObject
{

  friend class FIREBASE_CLASS;
  friend class FirebaseData;

public:
  FCMObject(){};
  ~FCMObject(){};

  template <typename T = const char *>
  void begin(T serverKey, SPI_ETH_Module *spi_ethernet_module = NULL)
  {
    FB_DEFAULT_DEBUG_PORT.println(FPSTR("This method was deprecated, please see examples/Messaging for how to use new methods."));
  }
  template <typename T = const char *>
  void addDeviceToken(T deviceToken) {}
  void removeDeviceToken(uint16_t index) {}
  void clearDeviceToken() {}
  template <typename T1 = const char *, typename T2 = const char *>
  void setNotifyMessage(T1 title, T2 body) {}
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
  void setNotifyMessage(T1 title, T2 body, T3 icon) {}
  template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *, typename T4 = const char *>
  void setNotifyMessage(T1 title, T2 body, T3 icon, T4 click_action) {}
  template <typename T1 = const char *, typename T2 = const char *>
  void addCustomNotifyMessage(T1 key, T2 value) {}
  void clearNotifyMessage() {}
  template <typename T = const char *>
  void setDataMessage(T jsonString) {}
  void setDataMessage(FirebaseJson &json) {}
  void clearDataMessage() {}
  template <typename T = const char *>
  void setPriority(T priority) {}
  template <typename T = const char *>
  void setCollapseKey(T key) {}
  void setTimeToLive(uint32_t seconds) {}
  template <typename T = const char *>
  void setTopic(T topic) {}
  const char *getSendResult() { return ""; };
};

#endif
#endif