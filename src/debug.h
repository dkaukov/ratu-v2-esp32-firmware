/*
 * debug.h
 *
 *  Created on: 15Nov.,2017
 *      Author: ziss
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <Arduino.h>
#include <stdio.h>

void __null_debug_transport(const char *) {};

typedef void (*debug_transport_t)(const char *);
debug_transport_t __debug_transport = __null_debug_transport;

debug_transport_t set_debug_transport(debug_transport_t new_transport) {
  debug_transport_t res = __debug_transport;
  __debug_transport = new_transport;
  return res;
}

#if defined(MECH_DEBUG)

int debug_log_printf(const char* format, ...) {
  static char loc_buf[64];
  char* temp = loc_buf;
  int len;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);
  len = vsnprintf(NULL, 0, format, arg);
  va_end(copy);
  if (len >= sizeof(loc_buf)) {
    temp = (char*)malloc(len + 1);
    if (temp == NULL) {
      return 0;
    }
  }
  vsnprintf(temp, len + 1, format, arg);
  __debug_transport(temp);
  va_end(arg);
  if (len >= sizeof(loc_buf)) {
    free(temp);
  }
  return len;
}

#define __ASSERT_USE_STDERR
#include <assert.h>

#if defined(ESP32)

#if defined(SYSLOG_SERVER)

#include "WiFi.h"
#include <Syslog.h>

WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, SYSLOG_DEVICE_HOSTNAME, SYSLOG_APP_NAME, LOG_KERN, SYSLOG_PROTO_BSD);
#if defined(SYSLOG_EARLY_LOG)
static bool __sysLogEnabled = true;
#else
static bool __sysLogEnabled = false;
#endif

#define _LOGE(tag, fmt, ...)                                           \
  {                                                                    \
    ESP_LOGE(tag, fmt, ##__VA_ARGS__);                                 \
    if (__sysLogEnabled)                                               \
      syslog.logf(LOG_ERR, ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGW(tag, fmt, ...)                                               \
  {                                                                        \
    ESP_LOGW(tag, fmt, ##__VA_ARGS__);                                     \
    if (__sysLogEnabled)                                                   \
      syslog.logf(LOG_WARNING, ARDUHAL_LOG_FORMAT(W, fmt), ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGI(tag, fmt, ...)                                            \
  {                                                                     \
    ESP_LOGI(tag, fmt, ##__VA_ARGS__);                                  \
    if (__sysLogEnabled)                                                \
      syslog.logf(LOG_INFO, ARDUHAL_LOG_FORMAT(I, fmt), ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGD(tag, fmt, ...)                                             \
  {                                                                      \
    ESP_LOGD(tag, fmt, ##__VA_ARGS__);                                   \
    if (__sysLogEnabled)                                                 \
      syslog.logf(LOG_DEBUG, ARDUHAL_LOG_FORMAT(D, fmt), ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGV(tag, fmt, ...)                                             \
  {                                                                      \
    ESP_LOGV(tag, fmt, ##__VA_ARGS__);                                   \
    if (__sysLogEnabled)                                                 \
      syslog.logf(LOG_DEBUG, ARDUHAL_LOG_FORMAT(V, fmt), ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }

#else

#define _LOGE(tag, fmt, ...)           \
  {                                    \
    ESP_LOGE(tag, fmt, ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGW(tag, fmt, ...)           \
  {                                    \
    ESP_LOGW(tag, fmt, ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGI(tag, fmt, ...)           \
  {                                    \
    ESP_LOGI(tag, fmt, ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGD(tag, fmt, ...)           \
  {                                    \
    ESP_LOGD(tag, fmt, ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }
#define _LOGV(tag, fmt, ...)           \
  {                                    \
    ESP_LOGV(tag, fmt, ##__VA_ARGS__); \
    debug_log_printf(ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__);       \
  }

#endif

inline void debugInit() {
}

void debugLoop() {
}

#elif defined(__AVR__)
static FILE debugout = {0};

static int debug_putchar(char c, FILE *stream) {
  if (__dbg_buff_ptr >= BUFF_SIZE) {
    icsc.send(ID_HAL100, (char)cmdDebug, __dbg_buff_ptr, &__dbg_buff[0]);
    __dbg_buff_ptr = 0;
  }
  __dbg_buff[__dbg_buff_ptr] = c;
  __dbg_buff_ptr++;
  return 1;
}

inline void debugInit() {
  fdev_setup_stream(&debugout, debug_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &debugout;
  stderr = &debugout;
}

void debugLoop() {
  if (__dbg_buff_ptr > 0) {
    icsc.send(ID_HAL100, (char)cmdDebug, __dbg_buff_ptr, &__dbg_buff[0]);
    __dbg_buff_ptr = 0;
  }
}

#else

inline void debugInit() {
}

void debugLoop() {
}

#endif

#else

inline void debugInit() {
}

inline void debugLoop() {
}

//#define assert(expression)
#define _LOGE(tag, fmt, ...)
#define _LOGW(tag, fmt, ...)
#define _LOGI(tag, fmt, ...)
#define _LOGD(tag, fmt, ...)
#define _LOGV(tag, fmt, ...)

#endif

#endif /* DEBUG_H_ */
