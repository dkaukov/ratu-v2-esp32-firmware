#pragma once

#include "Actuator.h"
#include "ActuatorDAC.h"

namespace Actuators {

class ActuatorDACGPIO : public ActuatorDAC {
protected:
  virtual void _digitalWrite(uint8_t pin, uint8_t val) override {
    digitalWrite(pin, val);
  };

  virtual void _pinMode(uint8_t pin, uint8_t mode) override {
    pinMode(pin, mode);
  };

public:
  ActuatorDACGPIO(const char *name,
                  const gpio8bit gpio,
                  const float min,
                  const float max)
      : ActuatorDAC(name,
                    gpio,
                    min,
                    max){};
};

} // namespace Actuators