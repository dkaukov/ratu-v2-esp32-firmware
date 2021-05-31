#pragma once

#include "ArduinoJson.h"
#include "config.h"
#include "core/Component.h"
#include "debug.h"
#include "device/ATU.h"
#include "sensor/SWRMeter.h"
#include <Arduino.h>
#include <MQTT.h>

namespace Network {

class SDR : public Device::TxTuneRequest_Observer, Core::Component {
private:
  uint8_t _trx = 0;
  uint8_t _drive = 100;
  String _toSdrTopic = "tci-mqtt-gatewayv2/events/to-sdr";

protected:
public:
  SDR() : Core::Component(Core::COMPONENT_CLASS_NETWORK){};

  virtual void setConfig(const JsonObject &doc) override {
    auto node = doc["sdr"];
    if (!node["tune"]["trx"].isNull()) {
      _trx = node["tune"]["trx"];
    }
    if (!node["tune"]["drive"].isNull()) {
      _drive = node["tune"]["drive"];
    }
    if (!node["tune"]["topic"].isNull()) {
      _toSdrTopic = node["tune"]["topic"].as<String>();
    }
  };

  virtual void init() override{};

  void notification(Device::TxTuneRequest request) {
    StaticJsonDocument<256> doc;
    JsonObject obj = doc.to<JsonObject>();
    if (request.tuneEnabled) {
      obj["cmd"] = "drive";
      obj["power"] = _drive;
      publish(_toSdrTopic, doc);
    }
    obj.clear();
    obj["cmd"] = "tune";
    obj["trx"] = _trx;
    obj["enabled"] = request.tuneEnabled;
    publish(_toSdrTopic, doc);
  };
};

SDR sdr;

} // namespace Network