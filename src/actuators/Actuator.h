#pragma once

#include "core/Component.h"
#include <Arduino.h>

namespace Actuators {

class Actuator : public Core::Component {
protected:
  const char *_name;
  int32_t _min = 0;
  int32_t _max = UINT32_MAX >> 1;

  virtual int32_t pctToValue(float pct) {
    return (_min + round((_max - _min) * constrain(pct, 0.0, 1.0)));
  };

public:
  Actuator(etl::message_router_id_t id, const char *name) : Core::Component(id), _name(name){};
  virtual void calibrate(bool restoreState = false){};
  virtual int32_t getValue() const { return 0; };
  virtual float getPhisicalValue() const { return 0; };
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

  virtual void onCommand(Core::command_type_t type, const JsonObject &doc) override {
    if (type == Core::COMMAND_TYPE_ACTUATE) {
      auto node = doc["actuator"][_name];
      if (!node["calibrate"].isNull()) {
        bool value = node["calibrate"];
        if (value) {
          _LOGD("actuator", "Calibrating actuator %s. Will restore value: %d", _name, getValue());
          calibrate(true);
        }
      }
      if (!node["value"].isNull()) {
        int32_t value = node["value"];
        if (value != getValue()) {
          setValue(value);
          _LOGD("actuator", "[V] Moving actuator %s to the %d", _name, value);
        }
      }
      if (!node["pct"].isNull()) {
        int32_t value = pctToValue(node["pct"]);
        if (value != getValue()) {
          setValue(value);
          _LOGD("actuator", "[P] Moving actuator %s to the %d", _name, value);
        }
      }
    }
  }
};

} // namespace Actuators
