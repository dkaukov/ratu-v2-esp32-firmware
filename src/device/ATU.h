#pragma once

#include "actuators/Actuator.h"
#include "core/Component.h"
#include "debug.h"
#include "sensor/SWRMeter.h"
#include <Arduino.h>

namespace Device {

class ATU : public Core::Component {
protected:
  Sensor::SWRMeter &_swrMeter;
  uint8_t _swrMeterCyclesCount = 10;

  float measureAndWait() {
    _swrMeter.startMeasurementCycle(_swrMeterCyclesCount);
    busyWait([this]() { return !_swrMeter.isReady(); });
    return _swrMeter.getTarget();
  }

  void stepAndWait(Actuators::Actuator &actuator, int16_t &step) {
    step = actuator.step(step);
    busyWait([&actuator]() { return !actuator.isReady(); });
  }

  float stepAndMeasure(Actuators::Actuator &actuator, int16_t &step) {
    stepAndWait(actuator, step);
    return measureAndWait();
  }

  virtual void optimise(Actuators::Actuator &actuator, int16_t step, float hysteresis) {
    uint16_t stepCount = 0;
    float prevStepMeasurement = measureAndWait();
    _LOGD("optimize", "started P(-1) = %f, step = %d", prevStepMeasurement, step);
    while (step != 0) {
      if (!_swrMeter.isInRange()) {
        _LOGD("optimize", "aborting: measurements not in range.");
        break;
      }
      float curStepMeasurement = stepAndMeasure(actuator, step);
      _LOGD("optimise", "[%d]: step=%d, %s->%f P(n-1)=%f, P(n)=%f", stepCount, step, actuator.getName(), actuator.getPhisicalValue(), prevStepMeasurement, curStepMeasurement);
      if ((prevStepMeasurement < curStepMeasurement) && (abs(prevStepMeasurement - curStepMeasurement) > hysteresis)) {
        step = -step;
        if (stepCount != 0) {
          if ((step / 2) == 0) {
            _LOGD("optimize", "analyse: wrong direction of last step, stepping back: step = %d", step);
            curStepMeasurement = stepAndMeasure(actuator, step);
          }
          step = step / 2;
        } else {
          _LOGD("optimize", "analyse: wrong direction of 1st step, reversing with same value: step = %d", step);
          curStepMeasurement = stepAndMeasure(actuator, step);
        }
      }
      prevStepMeasurement = curStepMeasurement;
      stepCount++;
    }
    _LOGD("optimize", "finished: P(%d)=%f", stepCount, prevStepMeasurement);
  }

public:
  ATU(etl::message_router_id_t id, Sensor::SWRMeter &swrMeter) : Core::Component(id), _swrMeter(swrMeter){};
  virtual void resetToDefaults(){};
  virtual void tune(){};

  virtual void onCommand(Core::command_type_t type, const JsonObject &doc) override {
    if (type == Core::COMMAND_TYPE_TUNE) {
      static bool lock = false;
      busyWait([this]() { return lock; });
      lock = true;
      if (!doc["config"].isNull()) {
        setConfig(doc["config"].as<JsonObject>());
      }
      busyWait([this]() { return !isReady(); });
      tune();
      lock = false;
    }
  };

  virtual bool isReady() const { return true; }
};

} // namespace Device