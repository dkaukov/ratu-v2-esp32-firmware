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

#define CLIENT_ID_TEMPLATE "ATU-%04X%08X"
#define CLIENT_ID_SIZE (sizeof(CLIENT_ID_TEMPLATE) + 5)

class MQTT : public Core::Component {
private:
  MQTTClient *_client;
  String _host;
  int _port;
  String _clientId;
  String _configurationTopic;
  String _statusTopic;
  String _stateTopic;
  String _commandTopic;

  void configureTopics() {
    _configurationTopic = String(MQTT_CONFIG_TOPIC_ROOT) + "/" + _clientId;
    _statusTopic = String(MQTT_DEVICE_TOPIC_ROOT) + "/" + _clientId + "/status";
    _stateTopic = String(MQTT_DEVICE_TOPIC_ROOT) + "/" + _clientId + "/state";
    _commandTopic = String(MQTT_DEVICE_TOPIC_ROOT) + "/" + _clientId + "/command";
  }

  void getClientId() {
    char clientId[CLIENT_ID_SIZE];
    uint64_t chipid = ESP.getEfuseMac();
    uint16_t chip = (uint16_t)(chipid >> 32);
    snprintf(clientId, CLIENT_ID_SIZE, CLIENT_ID_TEMPLATE, chip, (uint32_t)chipid);
    _clientId = String(clientId);
  }

  static std::string getTime() {
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    char buf[sizeof "2011-10-08T07:07:09.000Z"];
    strftime(buf, sizeof buf, "%FT%T", gmtime(&curTime.tv_sec));
    sprintf(buf, "%s.%dZ", buf, milli);
    return buf;
  }

  void setupMqttConnection() {
    StaticJsonDocument<1024> doc;
    doc["stateTopic"] = _stateTopic;
    doc["commandTopic"] = _commandTopic;
    doc["statusTopic"] = _statusTopic;
    doc["device"]["id"] = _clientId;
    doc["device"]["heapSize"] = ESP.getHeapSize();
    doc["device"]["sdkVersion"] = ESP.getSdkVersion();
    doc["device"]["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    doc["device"]["sketchMD5"] = ESP.getSketchMD5();
    doc["device"]["chipModel"] = ESP.getChipModel();
    String output;
    serializeJson(doc, output);
    _client->publish(_configurationTopic, output, true, 2);
    _client->publish(_stateTopic, "online", true, 0);
    _client->subscribe(_commandTopic + "/#");
  }

public:
  MQTT() : Core::Component(Core::COMPONENT_CLASS_NETWORK) {
    static MQTTClient client(1024);
    _client = &client;
    _host = MQTT_HOST_NAME;
    _port = 1883;
  };

  void messageReceived(String &topic, String &payload) {
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, payload);
    if (doc["cmd"] == "tune") {
      broadcastCommand(Core::COMMAND_TYPE_TUNE, doc);
    }
  };

  virtual void sendStatus() {
    StaticJsonDocument<1024> doc;
    getGlobalStatus(doc);
    String output;
    serializeJson(doc, output);
    _client->publish(_statusTopic, output);
  }

  virtual void init() override {
    _client->begin(_host.c_str(), _port, net);
    _client->onMessage([this](String &topic, String &payload) { messageReceived(topic, payload); });
    getClientId();
    configureTopics();
    _client->setKeepAlive(1);
    _client->setWill(_stateTopic.c_str(), "offline", true, 0);
    _LOGI("mqtt", "MQTT init mqtt://%s:%d", _host.c_str(), _port);
  };

  virtual void getStatus(JsonDocument &doc) const override {
    doc["system"]["timestamp"] = getTime();
    doc["system"]["freeHeap"] = ESP.getFreeHeap();
  };

  virtual void timer1000() override {
    if (!_client->connected()) {
      if (_client->connect(_clientId.c_str())) {
        setupMqttConnection();
        _LOGI("mqtt", "MQTT connected to %s:%d", _host.c_str(), _port);
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