#pragma once

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
  String _host;
  int _port;
  String _clientId;
  String _configurationTopic;
  String _statusTopic;
  String _commandTopic;
  int16_t _mqttReconnectCount = -1;
  String _sysLogHost;
  uint16_t _backoff = 0;

  void configureTopics() {
    _configurationTopic = String(MQTT_CONFIG_TOPIC_ROOT) + "/" + _clientId;
    _statusTopic = String(MQTT_DEVICE_TOPIC_ROOT) + "/" + _clientId + "/status";
    _commandTopic = String(MQTT_DEVICE_TOPIC_ROOT) + "/" + _clientId + "/command";
  }

  void getClientId() {
    _clientId = String(getDeviceId());
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

  virtual void getInfo(JsonObject &doc) const override {
    doc["commandTopic"] = _commandTopic;
    doc["statusTopic"] = _statusTopic;

    doc["device"]["id"] = _clientId;
    doc["device"]["heapSize"] = ESP.getHeapSize();
    doc["device"]["sdkVersion"] = ESP.getSdkVersion();
    doc["device"]["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    doc["device"]["sketchMD5"] = ESP.getSketchMD5();
    doc["device"]["chipModel"] = ESP.getChipModel();
    doc["device"]["hardware"] = HW_INFO;

    doc["device"]["wifi"]["ssid"] = WiFi.SSID();
    doc["device"]["wifi"]["bssid"] = WiFi.BSSIDstr();
    doc["device"]["wifi"]["ip"] = net.localIP().toString();
    doc["device"]["wifi"]["hostname"] = WiFi.getHostname();
    doc["device"]["wifi"]["mqttLocalPort"] = net.localPort();
    doc["device"]["wifi"]["macAddress"] = WiFi.macAddress();
  };

  void setupMqttConnection() {
    net.setNoDelay(true);
    StaticJsonDocument<1024> doc;
    JsonObject obj = doc.to<JsonObject>();
    getGlobalInfo(obj);
    String output;
    serializeJson(doc, output);
    _client->publish(_configurationTopic, output, true, 2);
    _client->subscribe(_commandTopic + "/#");
  }

public:
  MQTT() : Core::Component(Core::COMPONENT_CLASS_NETWORK) {
    static MQTTClient client(1460);
    _client = &client;
    _host = MQTT_HOST_NAME;
    _port = 1883;
  };

  void messageReceived(MQTTClient *client, char topic[], char bytes[], int length) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, bytes, length);
    if (!error) {
      doc.shrinkToFit();
      JsonObject obj = doc.as<JsonObject>();
      String cmd = doc["cmd"];
      broadcastCommand(parseCommand(cmd), obj);
      doc.clear();
    } else {
      _LOGE("mqtt", "MQTT command deserialisation failure: %s", error.c_str());
    }
  }

  virtual void onCommand(Core::command_type_t type, const JsonObject &doc) override {
    if (type == Core::COMMAND_TYPE_CONFIG) {
      setGlobalConfig(doc);
    } else if (type == Core::COMMAND_TYPE_RESTART) {
      ESP.restart();
    } else if (type == Core::COMMAND_TYPE_UNKNOWN) {
      _LOGE("mqtt", "MQTT unknown command: '%s'", doc["cmd"].as<String>().c_str());
    }
  };

  virtual void sendStatus() {
    StaticJsonDocument<1024> doc;
    JsonObject obj = doc.to<JsonObject>();
    getGlobalStatus(obj);
    String output;
    serializeJson(doc, output);
    _client->publish(_statusTopic, output);
  }

  void connect() {
    if (WiFi.status() == WL_CONNECTED) {
      if (_backoff == 0) {
        if (net.connect(_host.c_str(), _port, 1000) <= 0) {
          _backoff = 15;
        } else {
          if (_client->connect(_clientId.c_str(), true)) {
            if (_client->connected()) {
              _mqttReconnectCount++;
              setupMqttConnection();
              _LOGI("mqtt", "MQTT connected to %s:%d", _host.c_str(), _port);
            }
          } else {
            _backoff = 5;
          }
        }
      } else {
        _backoff--;
      }
    }
  }

  virtual void init() override {
    _client->begin(_host.c_str(), _port, net);
    _client->onMessageAdvanced([this](MQTTClient *client, char topic[], char bytes[], int length) { messageReceived(client, topic, bytes, length); });
    getClientId();
    configureTopics();
    _client->setKeepAlive(MQTT_KEEP_ALIVE_SEC);
    _client->setWill(_statusTopic.c_str(), "{\"atu\": {\"state\": \"offline\"}}", true, 0);
    _LOGI("mqtt", "MQTT init mqtt://%s:%d", _host.c_str(), _port);
  };

  virtual void getStatus(JsonObject &doc) const override {
    doc["system"]["timestamp"] = getTime();
    doc["system"]["freeHeap"] = ESP.getFreeHeap();
    doc["system"]["minFreeHeap"] = ESP.getMinFreeHeap();
    doc["system"]["maxAllocHeap"] = ESP.getMaxAllocHeap();
    doc["system"]["stackHighWaterMark"] = uxTaskGetStackHighWaterMark(NULL);
    doc["system"]["rssi"] = WiFi.RSSI();
    doc["system"]["mqtt-reconnects"] = _mqttReconnectCount;
    doc["system"]["upTime"] = (uint32_t)(esp_timer_get_time() / 1000 / 1000);
  };

  virtual void setConfig(const JsonObject &doc) override {
#if defined(SYSLOG_SERVER)
    auto node = doc["syslog"];
    if (!node["host"].isNull() && !node["port"].isNull()) {
      _sysLogHost = node["host"].as<String>();
      int port = node["port"];
      syslog.server(_sysLogHost.c_str(), port);
      _LOGI("mqtt", "Syslog %s:%d", _sysLogHost.c_str(), port);
    }
    if (!node["enabled"].isNull()) {
      __sysLogEnabled = node["enabled"];
      if (__sysLogEnabled) {
        _LOGI("mqtt", "Syslog enabled.");
      }
    }
#endif
  }

  virtual void timer1000() override {
    if (!_client->connected()) {
      connect();
    } else {
      sendStatus();
    }
  };

  virtual void onPublish(const char *topic, const JsonDocument &doc) override {
    String output;
    serializeJson(doc, output);
    _LOGD("mqtt", "MQTT publish: %s => %s", topic, output.c_str());
    _client->publish(topic, output);
  };

  virtual void loop() override {
    _client->loop();
  };
};

MQTT mqtt;

} // namespace Network