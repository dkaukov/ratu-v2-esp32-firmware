/*
 * debug.h
 *
 *  Created on: 15Nov.,2017
 *      Author: ziss
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>

#if defined(MECH_DEBUG)

#define __ASSERT_USE_STDERR
#include <assert.h>

#define BUFF_SIZE 8
char __dbg_buff[BUFF_SIZE];
uint8_t __dbg_buff_ptr = 0;

#if defined(ESP32)

#include "config.h"
#include <Syslog.h>

#if defined(NTP_SERVER)

WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME, LOG_KERN, SYSLOG_PROTO_BSD);

#define _LOGE(tag, fmt, ...)                                         \
  {                                                                  \
    ESP_LOGE(tag, fmt, ##__VA_ARGS__);                               \
    syslog.logf(LOG_ERR, ARDUHAL_LOG_FORMAT(E, fmt), ##__VA_ARGS__); \
  }
#define _LOGW(tag, fmt, ...)                                             \
  {                                                                      \
    ESP_LOGW(tag, fmt, ##__VA_ARGS__);                                   \
    syslog.logf(LOG_WARNING, ARDUHAL_LOG_FORMAT(W, fmt), ##__VA_ARGS__); \
  }
#define _LOGI(tag, fmt, ...)                                          \
  {                                                                   \
    ESP_LOGI(tag, fmt, ##__VA_ARGS__);                                \
    syslog.logf(LOG_INFO, ARDUHAL_LOG_FORMAT(I, fmt), ##__VA_ARGS__); \
  }
#define _LOGD(tag, fmt, ...)                                           \
  {                                                                    \
    ESP_LOGD(tag, fmt, ##__VA_ARGS__);                                 \
    syslog.logf(LOG_DEBUG, ARDUHAL_LOG_FORMAT(D, fmt), ##__VA_ARGS__); \
  }
#define _LOGV(tag, fmt, ...)                                           \
  {                                                                    \
    ESP_LOGV(tag, fmt, ##__VA_ARGS__);                                 \
    syslog.logf(LOG_DEBUG, ARDUHAL_LOG_FORMAT(V, fmt), ##__VA_ARGS__); \
  }

#else

#define _LOGE(tag, fmt, ...)           \
  {                                    \
    ESP_LOGE(tag, fmt, ##__VA_ARGS__); \
  }
#define _LOGW(tag, fmt, ...)           \
  {                                    \
    ESP_LOGW(tag, fmt, ##__VA_ARGS__); \
  }
#define _LOGI(tag, fmt, ...)           \
  {                                    \
    ESP_LOGI(tag, fmt, ##__VA_ARGS__); \
  }
#define _LOGD(tag, fmt, ...)           \
  {                                    \
    ESP_LOGD(tag, fmt, ##__VA_ARGS__); \
  }
#define _LOGV(tag, fmt, ...)           \
  {                                    \
    ESP_LOGV(tag, fmt, ##__VA_ARGS__); \
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

#define assert(expression)
#define _LOGE(tag, fmt, ...)
#define _LOGW(tag, fmt, ...)
#define _LOGI(tag, fmt, ...)
#define _LOGD(tag, fmt, ...)
#define _LOGV(tag, fmt, ...)

#endif

#endif /* DEBUG_H_ */