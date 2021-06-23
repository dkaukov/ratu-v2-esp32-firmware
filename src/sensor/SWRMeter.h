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

public:
  SWRMeter(etl::message_router_id_t id, const char *name) : Sensor(id, name){};

  virtual float getFwd() const { return _fwd; };
  virtual float getRfl() const { return _rfl; };
  virtual float getRho() const { return _rfl / _fwd; };
  virtual float getSWR() const {
    float p = getRho();
    return (1 + p) / (1 - p);
  };
  virtual float getTarget() const { return getRho(); };

  virtual bool isInRange() const {
    return true;
  }

  virtual void getStatus(JsonObject &doc) const override {
    auto node = doc["sensor"][_name];
    node["fwd"] = getFwd();
    node["rfl"] = getRfl();
    if (isInRange()) {
      node["rho"] = getRho();
      node["swr"] = getSWR();
      node["target"] = getTarget();
    }
  };
};

} // namespace Sensor