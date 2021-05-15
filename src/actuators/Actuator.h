#pragma once

#include "core/Component.h"
#include <Arduino.h>

namespace Actuators {

class Actuator : public Core::Component {
protected:
  int32_t _min = 0;
  int32_t _max = UINT32_MAX >> 1;

public:
  const char *_name;
  Actuator(etl::message_router_id_t id, const char *name) : Core::Component(id), _name(name){};
  virtual void calibrate(bool restoreState = false){};
  virtual int32_t getValue() { return 0; };
  virtual float getPhisicalValue() { return 0; };
  virtual int32_t setValue(int32_t pos) { return 0; };
  virtual bool isAtLimit() {
    return (getValue() >= _max) || (getValue() <= _min);
  };
  virtual bool isReady() { return false; }
  virtual int32_t step(int16_t step) { return setValue(getValue() + step); }
};

} // namespace Actuators
