#pragma once

#include "actuators/Actuator.h"
#include "core/Component.h"
#include "debug.h"
#include "sensor/SWRMeter.h"
#include <Arduino.h>

namespace Device {

class ATU : public Core::Component {
protected:
  Sensor::SWRMeter *_swrMeter;
  uint8_t _swrMeterCyclesCount = 10;

  virtual void optimise(Actuators::Actuator *chanel, int16_t step, float hysteresis) {
    uint16_t stepCount = 0;
    _swrMeter->startMeasurementCycle(_swrMeterCyclesCount);
    busyWait([this]() { return !_swrMeter->isReady(); });
    float interimRflPercent = _swrMeter->getPct();
    _LOGD("optimize", "optimize() - initial rflPwrPercent = %f\n", _swrMeter->getPct());
    while (step != 0) {
      if (_swrMeter->isInRange()) {
        _LOGD("optimize", "optimize() - aborting: measurements not in range.\n");
        break;
      }
      chanel->step(step);
      busyWait([chanel]() { return !chanel->isReady(); });
      _swrMeter->startMeasurementCycle(_swrMeterCyclesCount);
      busyWait([this]() { return !_swrMeter->isReady(); });
      if ((interimRflPercent < _swrMeter->getPct()) && (abs(interimRflPercent - _swrMeter->getPct()) > hysteresis)) {
        step = -step;
        if (stepCount != 0) {
          if ((step / 2) == 0) {
            chanel->step(step);
            _swrMeter->startMeasurementCycle(_swrMeterCyclesCount);
            busyWait([this]() { return !_swrMeter->isReady(); });
          }
          step = step / 2;
        } else {
          _LOGD("optimize", "optimize() - wrong direction of 1st step\n");
        }
      }
      interimRflPercent = _swrMeter->getPct();
      stepCount++;
    }
  }


public:
  ATU(etl::message_router_id_t id) : Core::Component(id){};
  virtual void resetToDefaults(){};
  virtual void tune(){};
};

} // namespace Device