#pragma once

#include "actuators/Actuator.h"
#include "config.h"
#include "core/Component.h"
#include "debug.h"
#include "sensor/SWRMeter.h"
#include <Arduino.h>
#include <MQTT.h>
#include <time.h>

namespace Network {

WiFiClient net;

class MQTT : public Core::Component {
private:
  MQTTClient *_client;
  char _ssid[23];

public:
  MQTT() : Core::Component(Core::COMPONENT_CLASS_NETWORK) {
    static MQTTClient client(1024);
    _client = &client;
    uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
    uint16_t chip = (uint16_t)(chipid >> 32);
    snprintf(_ssid, 23, "MCUDEVICE-%04X%08X", chip, (uint32_t)chipid);
  };

  void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
  };

  virtual void sendStatus() {
    StaticJsonDocument<1024> doc;
    getGlobalStatus(doc);
    String output;
    serializeJson(doc, output);
    _client->publish("ratu-v2/status", output);
  }

  virtual void init() override {
    _client->begin("mqtt.sphere.home", net);
    _client->onMessage([this](String &topic, String &payload) { messageReceived(topic, payload); });
    _LOGI("mqtt", "MQTT Init %s:%d", "mqtt.sphere.home", 1883);
  };

  static std::string getTime() {
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    char buf[sizeof "2011-10-08T07:07:09.000Z"];
    strftime(buf, sizeof buf, "%FT%T", gmtime(&curTime.tv_sec));
    sprintf(buf, "%s.%dZ", buf, milli);
    return buf;
  }

  virtual void getStatus(JsonDocument &doc) const override {
    doc["system"]["timestamp"] = getTime();
    doc["system"]["freeHeap"] = ESP.getFreeHeap();
    doc["system"]["heapSize"] = ESP.getHeapSize();
    doc["system"]["sdkVersion"] = ESP.getSdkVersion();
    doc["system"]["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    doc["system"]["sketchMD5"] = ESP.getSketchMD5();
    doc["system"]["chipModel"] = ESP.getChipModel();
  };

  virtual void timer1000() override {
    if (!_client->connected()) {
      if (_client->connect(_ssid)) {
        _LOGI("mqtt", "MQTT connected to %s:%d", "mqtt.sphere.home", 1883);
      };
    } else {
      sendStatus();
    }
  };

  virtual void loop() override {
    _client->loop();
  };
};

MQTT mqtt;

} // namespace Network