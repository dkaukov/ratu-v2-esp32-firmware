#pragma once

#include "Actuator.h"
#include "ActuatorDAC.h"

namespace Actuators {

class ActuatorDACGPIO : public ActuatorDAC {
protected:
  virtual void dacSetValue(uint8_t val) override {
    digitalWrite(_gpio0, (val & 0b00000001));
    digitalWrite(_gpio1, (val & 0b00000010));
    digitalWrite(_gpio2, (val & 0b00000100));
    digitalWrite(_gpio3, (val & 0b00001000));
    digitalWrite(_gpio4, (val & 0b00010000));
    digitalWrite(_gpio5, (val & 0b00100000));
    digitalWrite(_gpio6, (val & 0b01000000));
    digitalWrite(_gpio7, (val & 0b10000000));
  };

public:
  ActuatorDACGPIO(const char *name,
                  uint8_t gpio0,
                  uint8_t gpio1,
                  uint8_t gpio2,
                  uint8_t gpio3,
                  uint8_t gpio4,
                  uint8_t gpio5,
                  uint8_t gpio6,
                  uint8_t gpio7)
      : ActuatorDAC(name,
                    gpio0,
                    gpio1,
                    gpio2,
                    gpio3,
                    gpio4,
                    gpio5,
                    gpio6,
                    gpio7){};

  virtual void init() override {
    pinMode(_gpio0, OUTPUT);
    pinMode(_gpio1, OUTPUT);
    pinMode(_gpio2, OUTPUT);
    pinMode(_gpio3, OUTPUT);
    pinMode(_gpio4, OUTPUT);
    pinMode(_gpio5, OUTPUT);
    pinMode(_gpio6, OUTPUT);
    pinMode(_gpio7, OUTPUT);
  };
};

} // namespace Actuators