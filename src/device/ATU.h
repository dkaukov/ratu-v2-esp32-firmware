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

  float measureAndWait() {
    _swrMeter->startMeasurementCycle(_swrMeterCyclesCount);
    busyWait([this]() { return !_swrMeter->isReady(); });
    return _swrMeter->getPct();
  }

  void stepAndWait(Actuators::Actuator *chanel, int16_t step) {
    chanel->step(step);
    busyWait([chanel]() { return !chanel->isReady(); });
  }

  float stepAndMeasure(Actuators::Actuator *chanel, int16_t step) {
    stepAndWait(chanel, step);
    return measureAndWait();
  }

  virtual void optimise(Actuators::Actuator *chanel, int16_t step, float hysteresis) {
    uint16_t stepCount = 0;
    float prevStepMeasurement = measureAndWait();
    _LOGD("optimize", "started P(-1) = %f, step = %d", prevStepMeasurement, step);
    while (step != 0) {
      if (!_swrMeter->isInRange()) {
        _LOGD("optimize", "aborting: measurements not in range.");
        break;
      }
      float curStepMeasurement = stepAndMeasure(chanel, step);
      _LOGD("optimise", "[%d]: step=%d, %s->%f P(n-1)=%f, P(n)=%f", stepCount, step, chanel->_name, chanel->getPhisicalValue(), prevStepMeasurement, curStepMeasurement);
      if ((prevStepMeasurement < curStepMeasurement) && (abs(prevStepMeasurement - curStepMeasurement) > hysteresis)) {
        step = -step;
        if (stepCount != 0) {
          if ((step / 2) == 0) {
            _LOGD("optimize", "analyse: wrong direction of last step, stepping back: step = %d", step);
            curStepMeasurement = stepAndMeasure(chanel, step);
          }
          step = step / 2;
        } else {
          _LOGD("optimize", "analyse: wrong direction of 1st step, reversing with same value: step = %d", step);
          curStepMeasurement = stepAndMeasure(chanel, step);
        }
      }
      prevStepMeasurement = curStepMeasurement;
      stepCount++;
    }
    _LOGD("optimize", "finished: P(%d)=%f", stepCount, prevStepMeasurement);
  }

public:
  ATU(etl::message_router_id_t id) : Core::Component(id){};
  virtual void resetToDefaults(){};
  virtual void tune(){};
};

} // namespace Device