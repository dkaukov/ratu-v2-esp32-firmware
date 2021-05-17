#pragma once

#include "Actuator.h"

namespace Actuators {

class ActuatorDAC : public Actuator {
protected:
  const uint8_t _gpio0;
  const uint8_t _gpio1;
  const uint8_t _gpio2;
  const uint8_t _gpio3;
  const uint8_t _gpio4;
  const uint8_t _gpio5;
  const uint8_t _gpio6;
  const uint8_t _gpio7;

  uint8_t _value;
  uint8_t _delay;

  virtual void internalSetValue(uint8_t val){};

public:
  ActuatorDAC(const char *name,
              uint8_t gpio0,
              uint8_t gpio1,
              uint8_t gpio2,
              uint8_t gpio3,
              uint8_t gpio4,
              uint8_t gpio5,
              uint8_t gpio6,
              uint8_t gpio7)
      : Actuator(Core::COMPONENT_CLASS_ACTUATOR, name),
        _gpio0(gpio0),
        _gpio1(gpio1),
        _gpio2(gpio2),
        _gpio3(gpio3),
        _gpio4(gpio4),
        _gpio5(gpio5),
        _gpio6(gpio6),
        _gpio7(gpio7) {
    _min = 1;
    _max = 255;
    _delay = 15;
  };

  virtual int32_t setValue(int32_t pos) override {
    uint8_t val = constrain(pos, _min, _max);
    if ((val & _value) != 0) {
      internalSetValue(val);
    } else {
      internalSetValue(val | _value);
      delay(_delay);
      internalSetValue(val);
    }
    _value = val;
  };
  virtual int32_t getValue() const override { return _value; };
  virtual bool isReady() const override { return true; }
};

} // namespace Actuators