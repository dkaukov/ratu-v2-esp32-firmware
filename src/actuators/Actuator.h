#pragma once

#include "core/Component.h"
#include <Arduino.h>

namespace Actuators {

class Actuator : public Core::Component {
protected:
  const char *_name;
  int32_t _min = 0;
  int32_t _max = UINT32_MAX >> 1;

public:
  Actuator(etl::message_router_id_t id, const char *name) : Core::Component(id), _name(name){};
  virtual void calibrate(bool restoreState = false){};
  virtual int32_t getValue() const { return 0; };
  virtual float getPhisicalValue() const { return 0; };
  virtual int32_t setValue(int32_t pos) { return 0; };
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
};

} // namespace Actuators
