#pragma once

#include "Actuator.h"
#include "ActuatorDAC.h"
#include "Adafruit_MCP23017.h"

namespace Actuators {

class ActuatorDACMCP23017 : public ActuatorDAC {
protected:
  Adafruit_MCP23017 &_mcp;

  virtual void _digitalWrite(uint8_t pin, uint8_t val) override {
    _mcp.digitalWrite(pin, val);
  };

  virtual void _pinMode(uint8_t pin, uint8_t mode) override {
    _mcp.pinMode(pin, mode);
  };

public:
  ActuatorDACMCP23017(const char *name,
                      const gpio8bit gpio,
                      Adafruit_MCP23017 &mcp,
                      const float min,
                      const float max)
      : ActuatorDAC(name,
                    gpio,
                    min,
                    max),
        _mcp(mcp){};

  virtual void init() override {
    ActuatorDAC::init();
  };
};

} // namespace Actuators