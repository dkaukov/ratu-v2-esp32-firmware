#pragma once

#include "core/Component.h"
#include "debug.h"
#include <Arduino.h>

namespace Sensor {

class Sensor : public Core::Component {
protected:
  uint8_t _cnt = 0;
  const char *_name;

public:
  Sensor(etl::message_router_id_t id, const char *name) : Core::Component(id), _name(name){};
  virtual void startMeasurementCycle(uint8_t cnt) { _cnt = cnt; };
  virtual bool isReady() const { return _cnt == 0; };
};

} // namespace Sensor