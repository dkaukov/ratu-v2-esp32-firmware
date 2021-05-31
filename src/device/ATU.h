#pragma once

#include "actuators/Actuator.h"
#include "core/Component.h"
#include "debug.h"
#include "etl/observer.h"
#include "sensor/SWRMeter.h"
#include <Arduino.h>

namespace Device {

typedef enum {
  ATU_STATE_OFFLINE,
  ATU_STATE_READY,
  ATU_STATE_BUSY,
} atu_state_type_t;

struct TxTuneRequest {
  bool tuneEnabled;
};

typedef etl::observer<TxTuneRequest> TxTuneRequest_Observer;

class ATU : public Core::Component, etl::observable<TxTuneRequest_Observer, 1> {
protected:
  Sensor::SWRMeter &_swrMeter;
  uint8_t _swrMeterCyclesCount = 10;
  uint8_t _waitTickTimeout;

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

  void moveAndWait(Actuators::Actuator &actuator, uint32_t pos) {
    actuator.setValue(pos);
    busyWait([&actuator]() { return !actuator.isReady(); });
  }

  float moveAndMeasure(Actuators::Actuator &actuator, uint32_t pos) {
    moveAndWait(actuator, pos);
    return measureAndWait();
  }

  virtual float optimise(Actuators::Actuator &actuator, int16_t step, float hysteresis) {
    uint16_t stepCount = 0;
    float prevStepMeasurement = measureAndWait();
    _LOGD("optimize", "started P(-1) = %f, step = %d", prevStepMeasurement, step);
    _LOGD("optimize", "Phase 1: finding interval by Svenn method.");
    while (step != 0) {
      if (!_swrMeter.isInRange()) {
        _LOGD("optimize", "aborting: measurements not in range.");
        break;
      }
      float curStepMeasurement = stepAndMeasure(actuator, step);
      _LOGD("optimise", "Sv:[%d]: step=%d, %s->%f P(n-1)=%f, P(n)=%f", stepCount, step, actuator.getName(), actuator.getPhisicalValue(), prevStepMeasurement, curStepMeasurement);
      if ((prevStepMeasurement < curStepMeasurement) && (abs(prevStepMeasurement - curStepMeasurement) > hysteresis)) {
        step = -step;
        if (stepCount != 0) {
          _LOGD("optimize", "analyse: found interval, switching to the binary search: step = %d", step);
        } else {
          _LOGD("optimize", "analyse: wrong direction of 1st step, reversing with same value: step = %d", step);
          curStepMeasurement = stepAndMeasure(actuator, step);
        }
      } else {
        step = step * 2;
      }
      prevStepMeasurement = curStepMeasurement;
      stepCount++;
    }
    _LOGD("optimize", "Svenn finished: P(%d)=%f, step=%d", stepCount, prevStepMeasurement, step);
    return optimiseFibonacci(actuator, min(actuator.getValue(), actuator.getValue() + step), max(actuator.getValue(), actuator.getValue() + step), stepCount);
  }

  virtual float optimiseFibonacci(Actuators::Actuator &actuator, uint32_t a, uint32_t b, uint16_t &stepCount) {
    _LOGD("optimize", "Phase 2: Fibonacci search of the minimum.");
    uint32_t x1 = a + round(0.382 * (b - a));
    uint32_t x2 = b - round(0.382 * (b - a));
    float A = moveAndMeasure(actuator, x1);
    float B = moveAndMeasure(actuator, x2);
    stepCount = stepCount + 2;
    while (true) {
      if (A < B) {
        b = x2;
        if ((b - a) <= 1) {
          break;
        }
        x2 = x1;
        B = A;
        x1 = a + round(0.382 * (b - a));
        A = moveAndMeasure(actuator, x1);
        stepCount++;
      } else {
        a = x1;
        if ((b - a) <= 1) {
          break;
        }
        x1 = x2;
        A = B;
        x2 = b - round(0.382 * (b - a));
        B = moveAndMeasure(actuator, x2);
        stepCount++;
      }
    }
    _LOGD("optimize", "Fibonacci finished: P(%d)=%f", stepCount, A);
    return A;
  }

  void turnOnTrx() {
    TxTuneRequest txTuneRequest = {tuneEnabled : true};
    notify_observers(txTuneRequest);
  }

  void turnOffTrx() {
    TxTuneRequest txTuneRequest = {tuneEnabled : false};
    notify_observers(txTuneRequest);
  }

public:
  ATU(etl::message_router_id_t id, Sensor::SWRMeter &swrMeter) : Core::Component(id), _swrMeter(swrMeter){};
  virtual void resetToDefaults(){};
  virtual void tune(){};

  virtual void onCommand(Core::command_type_t type, const JsonObject &doc) override {
    if (type == Core::COMMAND_TYPE_TUNE) {
      static bool lock = false;
      _LOGD("atu", "Tune, waiting for lock.");
      busyWait([this]() { return lock; });
      lock = true;
      if (!doc["config"].isNull()) {
        setGlobalConfig(doc["config"].as<JsonObject>());
      }
      _LOGD("atu", "Tune, waiting for isReady().");
      busyWait([this]() { return !isReady(); });
      turnOnTrx();
      _waitTickTimeout = 10;
      _LOGD("atu", "Tune, waiting for swrMeter.isInRange().");
      busyWait([this]() { return !(_swrMeter.isInRange() || (_waitTickTimeout == 0)); });
      if (_swrMeter.isInRange()) {
        tune();
      } else {
        _LOGE("atu", "Timeout waiting for swrMeter.isInRange(), aborting.");
      }
      turnOffTrx();
      lock = false;
    }
  };

  virtual bool isReady() const { return true; }

  virtual atu_state_type_t getState() const {
    if (isReady()) {
      return ATU_STATE_READY;
    }
    return ATU_STATE_BUSY;
  }

  virtual void getStatus(JsonObject &doc) const override {
    auto node = doc["atu"];
    switch (getState()) {
    case ATU_STATE_OFFLINE:
      node["state"] = "offline";
      break;
    case ATU_STATE_BUSY:
      node["state"] = "busy";
      break;
    case ATU_STATE_READY:
      node["state"] = "ready";
      break;
    default:
      break;
    }
  }

  virtual void registerObserver(TxTuneRequest_Observer &observer) {
    add_observer(observer);
  }

  virtual void timer250() override {
    if (_waitTickTimeout) {
      _waitTickTimeout--;
    }
  }
};

} // namespace Device