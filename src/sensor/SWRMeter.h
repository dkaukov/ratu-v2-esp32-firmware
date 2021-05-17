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

  virtual void getStatus(JsonDocument &doc) const override {
    doc["sensor"][_name]["fwd"] = getFwd();
    doc["sensor"][_name]["rfl"] = getRfl();
    doc["sensor"][_name]["pct"] = getPct();
    doc["sensor"][_name]["swr"] = getSWR();
    doc["sensor"][_name]["target"] = getTarget();
  };

  virtual void setConfig(const JsonDocument &doc) override {
    if (!doc["sensor"][_name]["minFwd"].isUndefined()) {
      _minFwd = doc["sensor"][_name]["minFwd"];
    }
    if (!doc["sensor"][_name]["minRfl"].isUndefined()) {
      _minRfl = doc["sensor"][_name]["minRfl"];
    }
  };
};

} // namespace Sensor