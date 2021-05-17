#define MECH_DEBUG 1
#define ETL_NO_STL
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

#include "ArduinoJson.h"
#include "WiFi.h"
#include "config.h"
#include "core/Component.h"
#include "debug.h"
#include "hardware/TMatchWithRelays.h"
//#include "hardware/TMatchWithStepprs.h"
#include "lwip/apps/sntp.h"
#include "secrets.h"
#include <Arduino.h>

Core::ComponentManager mgr;
Hardware::TMatchWithRelays atu;

void setupWiFi() {
  btStop();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.setAutoReconnect(true);
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char *)NTP_SERVER);
  sntp_init();
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    _LOGI("main", "WiFi connected!");
    _LOGI("main", "IP address: %s", WiFi.localIP().toString().c_str());
    _LOGI("main", "IP hostname: %s", WiFi.getHostname());
  } else {
    _LOGE("main", "Wifi connection failed!");
    ESP.restart();
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(250000);
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  setupWiFi();
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  debugInit();
  atu.init();
  mgr.init();
}

void loop() {
  debugLoop();
  mgr.loop();
}