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

typedef enum {
  TCI_VER_14,
  TCI_VER_15,
} sdr_version_type_t;

class SDR : public Device::TxTuneRequest_Observer, Core::Component {
private:
  uint8_t _trx = 0;
  int8_t _drive = 100;
  String _toSdrTopic = "tci-mqtt-gatewayv2/events/to-sdr";
  sdr_version_type_t _ver = TCI_VER_14;

  sdr_version_type_t stringToTCIVersion(const String &val) const {
    if (val == "TCI_VER_14") {
      return TCI_VER_14;
    }
    return TCI_VER_15;
  }

  const char *tciVersionToString(const sdr_version_type_t val) const {
    switch (val) {
    case TCI_VER_14:
      return "TCI_VER_14";
    case TCI_VER_15:
      return "TCI_VER_15";
    default:
      return "";
    }
  }

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
    if (!node["tune"]["tune_drive"].isNull()) {
      _drive = node["tune"]["tune_drive"];
    }
    if (!node["tune"]["topic"].isNull()) {
      _toSdrTopic = node["tune"]["topic"].as<String>();
    }
    if (!node["tci_version"].isNull()) {
      _ver = stringToTCIVersion(node["tci_version"]);
    }
  };

  virtual void init() override{};

  void notification(Device::TxTuneRequest request) {
    StaticJsonDocument<256> doc;
    JsonObject obj = doc.to<JsonObject>();
    if (request.tuneEnabled && (_drive >= 0) && (_drive <= 100)) {
      obj["cmd"] = "tune_drive";
      obj["power"] = _drive;
      if (_ver == TCI_VER_15) {
        obj["trx"] = _trx;
      }
      publish(_toSdrTopic, doc);
    }
    obj.clear();
    {
      obj["cmd"] = "tune";
      obj["trx"] = _trx;
      obj["enabled"] = request.tuneEnabled;
      publish(_toSdrTopic, doc);
    }
  };
};

SDR sdr;

} // namespace Network