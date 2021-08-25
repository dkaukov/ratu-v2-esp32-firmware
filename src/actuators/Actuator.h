#pragma once

#include "core/Component.h"
#include <Arduino.h>

namespace Actuators {

class Actuator : public Core::Component {
protected:
  const char *_name;
  int32_t _min = 0;
  int32_t _max = UINT32_MAX >> 1;
  float _a = 1.0;
  float _b = 0;
  float __a = 1.0;
  float __b = 0;
  bool _inverted = false;

  virtual int32_t pctToValue(float pct) {
    if (_inverted) {
      return (_min + round((_max - _min) * (1.0 - constrain(pct, 0.0, 1.0))));
    } else {
      return (_min + round((_max - _min) * constrain(pct, 0.0, 1.0)));
    }
  };

  virtual int32_t phToValue(float x) {
    return round(__a * x + __b);
  };

public:
  Actuator(etl::message_router_id_t id, const char *name) : Core::Component(id), _name(name){};
  virtual void calibrate(bool restoreState = false){};
  virtual int32_t getValue() const { return 0; };
  virtual int32_t getTargetValue() const { return getValue(); };
  int32_t getMin() const { return _min; };
  int32_t getMax() const { return _max; };
  virtual float getPhisicalValue() const { return _a * getValue() + _b; };
  virtual int32_t setValue(int32_t pos) { return 0; };
  virtual int32_t setPct(float pct) { return setValue(pctToValue(pct)); };
  virtual bool isAtLimit() const {
    return (getValue() >= _max) || (getValue() <= _min);
  };
  virtual bool isReady() const { return false; }
  virtual int32_t step(int16_t step) {
    if (step == 0) {
      return 0;
    }
    return setValue(getValue() + step);
  }
  const char *getName() const {
    return _name;
  }

  virtual void getStatus(JsonObject &doc) const override {
    doc["actuator"][_name]["value"] = getValue();
    doc["actuator"][_name]["phValue"] = getPhisicalValue();
    doc["actuator"][_name]["isReady"] = isReady();
    doc["actuator"][_name]["isAtLimit"] = isAtLimit();
  };

  virtual void getInfo(JsonObject &doc) const override {
    doc["actuator"][_name]["min"] = _min;
    doc["actuator"][_name]["max"] = _max;
    doc["actuator"][_name]["minPh"] = _a * _min + _b;
    doc["actuator"][_name]["maxPh"] = _a * _max + _b;
  };

  virtual void onCommand(Core::command_type_t type, const JsonObject &doc) override {
    if (type == Core::COMMAND_TYPE_ACTUATE) {
      auto node = doc["actuator"][_name];
      if (!node["calibrate"].isNull()) {
        bool value = node["calibrate"];
        if (value) {
          _LOGD("actuator", "Calibrating actuator %s. Will restore value: %d", _name, getTargetValue());
          calibrate(true);
        }
      }
      if (!node["value"].isNull()) {
        int32_t value = node["value"];
        if (value != getTargetValue()) {
          setValue(value);
          _LOGD("actuator", "[V] Moving actuator %s to the %d", _name, getTargetValue());
        }
      }
      if (!node["pct"].isNull()) {
        int32_t value = pctToValue(node["pct"]);
        if (value != getTargetValue()) {
          setValue(value);
          _LOGD("actuator", "[P] Moving actuator %s to the %d", _name, getTargetValue());
        }
      }
      if (!node["phValue"].isNull()) {
        int32_t value = phToValue(node["phValue"]);
        if (value != getTargetValue()) {
          setValue(value);
          _LOGD("actuator", "[F] Moving actuator %s to the %d", _name, getTargetValue());
        }
      }
    }
  }
};

} // namespace Actuators
