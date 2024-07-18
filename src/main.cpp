#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

#if not defined(HARDAWARE_TYPE_TMATCH_WITH_RELAYS) && not defined(HARDAWARE_TYPE_TMATCH_WITH_STEPPERS) && not defined(HARDAWARE_TYPE_LMATCH_WITH_STEPPERS)
#define HARDAWARE_TYPE_TMATCH_WITH_RELAYS
#endif

#define IP_CONFIGURATION_TYPE_DHCP 0
#define IP_CONFIGURATION_TYPE_STATIC 1

const char *getDeviceId();


#include "ArduinoJson.h"
#include "WiFi.h"
#include "config.h"
#include "core/Component.h"
#include "debug.h"
#include "device/LMatch.h"
#include "device/TMatch.h"
#if defined(HARDAWARE_TYPE_TMATCH_WITH_RELAYS)
#include "hardware/TMatchWithRelays.h"
#elif defined(HARDAWARE_TYPE_TMATCH_WITH_STEPPERS)
#include "hardware/TMatchWithStepprs.h"
#elif defined(HARDAWARE_TYPE_LMATCH_WITH_STEPPERS)
#include "hardware/LMatchWithStepprs.h"
#endif
#include "lwip/apps/sntp.h"
#include "network/MQTT.h"
#include "network/SDR.h"
#include "network/WEB.h"
#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <esp_task_wdt.h>

#define CLIENT_ID_TEMPLATE "ATU-%06X"
#define CLIENT_ID_SIZE (sizeof(CLIENT_ID_TEMPLATE) + 5)

Core::ComponentManager mgr;
String deviceId;

void initDeviceId() {
#if defined(DEVICE_ID)
  deviceId = DEVICE_ID;
#else
  char clientId[CLIENT_ID_SIZE];
  uint32_t nic = ESP.getEfuseMac() >> 24;
  uint32_t chipid = nic & 0xFF;
  chipid = chipid << 8;
  nic = nic >> 8;
  chipid = chipid | (nic & 0xFF);
  chipid = chipid << 8;
  nic = nic >> 8;
  chipid = chipid | (nic & 0xFF);
  snprintf(clientId, CLIENT_ID_SIZE, CLIENT_ID_TEMPLATE, chipid);
  deviceId = String(clientId);
#endif
}

void printEnvironment() {
  _LOGI("main", "---");
  _LOGI("main", "Hostname: %s", getDeviceId());
  _LOGI("main", "MAC: %s", WiFi.macAddress().c_str());
  _LOGI("main", "Heap Size: %d", ESP.getHeapSize());
  _LOGI("main", "SDK Version: %s", ESP.getSdkVersion());
  _LOGI("main", "CPU Freq: %d", ESP.getCpuFreqMHz());
  _LOGI("main", "Sketch MD5: %s", ESP.getSketchMD5().c_str());
  _LOGI("main", "Chip model: %s", ESP.getChipModel());
  _LOGI("main", "Hardware: %s", HW_INFO);
  _LOGI("main", "---");
}

const char *getDeviceId() {
  return deviceId.c_str();
}

void WiFiSTAConnect() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
#if (!defined(IP_CONFIGURATION_TYPE) || (IP_CONFIGURATION_TYPE == IP_CONFIGURATION_TYPE_DHCP))
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
#elif (defined(IP_CONFIGURATION_TYPE) && (IP_CONFIGURATION_TYPE == IP_CONFIGURATION_TYPE_STATIC))
#define IP(name, value) IPAddress name(value);
  IP(local_ip, IP_CONFIGURATION_ADDRESS);
  IP(local_mask, IP_CONFIGURATION_MASK);
  IP(gw, IP_CONFIGURATION_GW);
  WiFi.config(local_ip, gw, local_mask);
#else
#error "Incorrect IP_CONFIGURATION_TYPE."
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname(getDeviceId());
  WiFi.setAutoReconnect(true);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  _LOGI("main", "Connected to AP successfully!");
}

void wiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  WiFiSTAConnect();
}

void wiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  _LOGI("main", "WiFi connected!");
  _LOGI("main", "IP address: %s", WiFi.localIP().toString().c_str());
  _LOGI("main", "IP hostname: %s", WiFi.getHostname());
}

void setupWiFi() {
  btStop();
  WiFiSTAConnect();
  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(wiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(wiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
#if defined(NTP_SERVER)
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char *)NTP_SERVER);
  sntp_init();
#endif
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    _LOGE("main", "Wifi connection failed!");
  }
}

void setup() {
  initDeviceId();
  printEnvironment();
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  setupWiFi();
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  debugInit();
  ArduinoOTA.setMdnsEnabled(true);
  ArduinoOTA.setHostname(getDeviceId());
  ArduinoOTA.begin();
  Network::mqtt.init();
  Network::sdr.init();
  Network::web.init();
  Hardware::atu.init();
  Hardware::atu.registerObserver(Network::sdr);
  mgr.init();
}

void loop() {
  esp_task_wdt_reset();
  debugLoop();
  ArduinoOTA.handle();
  mgr.loop();
}