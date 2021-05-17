#pragma once

#include "Actuator.h"

namespace Actuators {

#define DAC_BIT_COUNT 8

typedef struct gpio8bit {
  uint8_t pin[DAC_BIT_COUNT];
} gpio8bit;

class ActuatorDAC : public Actuator {
protected:
  const gpio8bit _gpio;

  uint8_t _value;
  uint8_t _delay;

  void dacSetValue(uint8_t val) {
    for (uint8_t i = 0; i < DAC_BIT_COUNT; i++) {
      _digitalWrite(_gpio.pin[0], (val & (1 << i)));
    }
  };

  virtual void _digitalWrite(uint8_t pin, uint8_t val){};
  virtual void _pinMode(uint8_t pin, uint8_t mode){};

public:
  ActuatorDAC(const char *name,
              const gpio8bit gpio)
      : Actuator(Core::COMPONENT_CLASS_ACTUATOR, name),
        _gpio(gpio) {
    _min = 1;
    _max = 255;
    _delay = 15;
  };

  virtual int32_t setValue(int32_t pos) override {
    uint8_t val = constrain(pos, _min, _max);
    uint8_t oldVal = _value;
    if ((val & _value) != 0) {
      dacSetValue(val);
    } else {
      dacSetValue(val | _value);
      delay(_delay);
      dacSetValue(val);
    }
    _value = val;
    return val - oldVal;
  };

  virtual int32_t getValue() const override { return _value; };

  virtual bool isReady() const override { return true; }

  virtual void init() override {
    for (uint8_t i = 0; i < DAC_BIT_COUNT; i++) {
      _pinMode(_gpio.pin[i], OUTPUT);
    }
  };
};

} // namespace Actuators