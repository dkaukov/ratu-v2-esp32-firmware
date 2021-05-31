//#define MECH_DEBUG 1
#define ETL_NO_STL
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

const char *getDeviceId();

#include "ArduinoJson.h"
#include "WiFi.h"
#include "config.h"
#include "core/Component.h"
#include "debug.h"
#include "hardware/TMatchWithRelays.h"
//#include "hardware/TMatchWithStepprs.h"
#include "lwip/apps/sntp.h"
#include "network/MQTT.h"
#include "network/SDR.h"
#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>

#define CLIENT_ID_TEMPLATE "ATU-%04X%08X"
#define CLIENT_ID_SIZE (sizeof(CLIENT_ID_TEMPLATE) + 5)

Core::ComponentManager mgr;
Hardware::TMatchWithRelays atu;
String deviceId;

void InitDeviceId() {
#if defined(DEVICE_ID)
  deviceId = DEVICE_ID;
#else
  char clientId[CLIENT_ID_SIZE];
  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(clientId, CLIENT_ID_SIZE, CLIENT_ID_TEMPLATE, chip, (uint32_t)chipid);
  deviceId = String(clientId);
#endif
}

const char *getDeviceId() {
  return deviceId.c_str();
}

void WiFiSTAConnect() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname(getDeviceId());
  WiFi.setAutoReconnect(true);
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
  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(wiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.onEvent(wiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
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
  InitDeviceId();
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(250000);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  setupWiFi();
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  debugInit();
  ArduinoOTA.setMdnsEnabled(true);
  ArduinoOTA.setHostname(getDeviceId());
  ArduinoOTA.begin();
  Network::mqtt.init();
  Network::sdr.init();
  atu.init();
  atu.registerObserver(Network::sdr);
  mgr.init();
}

void loop() {
  debugLoop();
  ArduinoOTA.handle();
  mgr.loop();
}