#pragma once

#include "ArduinoJson.h"
#include "Sensor.h"
#include "debug.h"
#include <Arduino.h>

namespace Sensor {

class SWRMeter : public Sensor {
protected:
  float _fwd;
  float _rfl;
  float _minFwd = 10.0;
  float _minRfl = 4.0;

public:
  SWRMeter(etl::message_router_id_t id, const char *name) : Sensor(id, name){};

  virtual float getFwd() const { return _fwd; };
  virtual float getRfl() const { return _rfl; };
  virtual float getPct() const { return _rfl / _fwd; };
  virtual float getSWR() const {
    float p = getPct();
    return (1 + p) / (1 - p);
  };
  virtual float getTarget() const { return getPct(); };

  virtual bool isInRange() const {
    return (_fwd > _minFwd) && (_rfl > _minRfl);
  }

  virtual void getStatus(JsonObject &doc) const override {
    auto node = doc["sensor"][_name];
    node["fwd"] = getFwd();
    node["rfl"] = getRfl();
    node["pct"] = getPct();
    node["swr"] = getSWR();
    node["target"] = getTarget();
  };

  virtual void setConfig(const JsonObject &doc) override {
    auto node = doc["sensor"][_name];
    if (!node["minFwd"].isNull()) {
      _minFwd = node["minFwd"];
    }
    if (!node["minRfl"].isNull()) {
      _minRfl = node["minRfl"];
    }
  };
};

} // namespace Sensor