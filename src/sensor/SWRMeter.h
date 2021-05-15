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

  virtual float getFwd() { return _fwd; };
  virtual float getRfl() { return _rfl; };
  virtual float getPct() { return _rfl / _fwd; };
  virtual float getSWR() {
    float p = getPct();
    return (1 + p) / (1 - p);
  };

  virtual bool isInRange() {
    return (_fwd > _minFwd) && (_rfl > _minRfl);
  }

  virtual void getStatus(JsonDocument &doc) {
    doc["sensor"][*_name]["fwd"] = getFwd();
    doc["sensor"][*_name]["rfl"] = getRfl();
    doc["sensor"][*_name]["pct"] = getPct();
    doc["sensor"][*_name]["swr"] = getSWR();
  };

  virtual void setConfig(const JsonDocument &doc) {
    if (!doc["sensor"][*_name]["minFwd"].isUndefined()) {
      _minFwd = doc["sensor"][*_name]["minFwd"];
    }
    if (!doc["sensor"][*_name]["minRfl"].isUndefined()) {
      _minRfl = doc["sensor"][*_name]["minRfl"];
    }
  };
};

} // namespace Sensor