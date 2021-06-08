#pragma once

#include "actuators/Actuator.h"
#include "core/Component.h"
#include "debug.h"
#include "etl/observer.h"
#include "sensor/SWRMeter.h"
#include <Arduino.h>

namespace Device {

#define FIB_INTERVAL_RATIO 0.381966011 // 1.0 - (sqrt(5.0) - 1.0) / 2.0
#define FIB_MIN_INTERVAL 4.236067977   // 1.0 / (1.0 - 2.0 * FIB_INTERVAL_RATIO)

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
  uint8_t _swrMeterCyclesCount = 8;
  uint8_t _waitTickTimeout;

  float measureAndWait() {
    _swrMeter.startMeasurementCycle(_swrMeterCyclesCount);
    busyWait([this]() { return !_swrMeter.isReady(); });
    return _swrMeter.getTarget();
  }

  void stepAndWait(Actuators::Actuator &actuator, int16_t step) {
    actuator.step(step);
    busyWait([&actuator]() { return !actuator.isReady(); });
  }

  float stepAndMeasure(Actuators::Actuator &actuator, int16_t step) {
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
    bool weHadImprovement = false;
    _LOGD("optimize", "started P(-1) = %f, step = %d, %s->%f(%d)", prevStepMeasurement, step, actuator.getName(), actuator.getPhisicalValue(), actuator.getValue());
    _LOGD("optimize", "Phase 1: finding interval by Svenn method.");
    while (step != 0) {
      if (!_swrMeter.isInRange()) {
        _LOGD("optimize", "aborting: measurements not in range.");
        break;
      }
      float curStepMeasurement = stepAndMeasure(actuator, step);
      _LOGD("optimise", "Sv:[%d]: step=%d, %s->%f(%d) P(n-1)=%f, P(n)=%f", stepCount, step, actuator.getName(), actuator.getPhisicalValue(), actuator.getValue(), prevStepMeasurement, curStepMeasurement);
      if (((prevStepMeasurement < curStepMeasurement) && (abs(prevStepMeasurement - curStepMeasurement) > hysteresis)) || actuator.isAtLimit()) {
        step = -step;
        if (stepCount != 0) {
          if (!weHadImprovement) {
            _LOGD("optimize", "analyse: Looks like we too close to minimum, increasing interval to refine search.");
            step = step * 2;
          } else {
            step = step + step / 2;
          }
          stepCount++;
          _LOGD("optimize", "analyse: found interval, switching to the binary search: step = %d", step);
          break;
        } else {
          _LOGD("optimize", "analyse: wrong direction of 1st step, reversing with same value: step = %d", step);
          // curStepMeasurement = stepAndMeasure(actuator, step);
          curStepMeasurement = prevStepMeasurement;
          step = step * 2;
        }
      } else {
        step = step * 2;
        weHadImprovement = true;
      }
      prevStepMeasurement = curStepMeasurement;
      stepCount++;
    }
    _LOGD("optimize", "Svenn finished: P(%d)=%f, step=%d", stepCount, prevStepMeasurement, step);
    return optimiseFibonacci(actuator, constrain(min(actuator.getValue(), actuator.getValue() + step), actuator.getMin(), actuator.getMax()),
                             constrain(max(actuator.getValue(), actuator.getValue() + step), actuator.getMin(), actuator.getMax()), stepCount);
  }

  virtual float optimiseFibonacci(Actuators::Actuator &actuator, float a, float b, uint16_t &stepCount) {
    _LOGD("optimize", "Phase 2: Fibonacci search of the minimum. a=%f, b=%f", a, b);
    if ((b - a) <= FIB_MIN_INTERVAL) {
      _LOGD("optimize", "Fib: Interval is to short, short-circuiting to linear search.");
      return optimiseLinear(actuator, a, b, stepCount);
    }
    float x1 = a + FIB_INTERVAL_RATIO * (b - a);
    float x2 = b - FIB_INTERVAL_RATIO * (b - a);
    float A = moveAndMeasure(actuator, round(x1));
    float ap = actuator.getPhisicalValue();
    float B = moveAndMeasure(actuator, round(x2));
    float bp = actuator.getPhisicalValue();
    stepCount = stepCount + 2;
    while (true) {
      _LOGD("optimise", "Fib:[%d]: int:[%f-%f] Pa(%f)=%f, Pb(%f)=%f [%s->%f, %s->%f]", stepCount, a, b, x1, A, x2, B, actuator.getName(), ap, actuator.getName(), bp);
      if (A < B) {
        b = x2;
        if ((b - a) <= FIB_MIN_INTERVAL) {
          break;
        }
        x2 = x1;
        B = A;
        x1 = a + FIB_INTERVAL_RATIO * (b - a);
        A = moveAndMeasure(actuator, round(x1));
        ap = actuator.getPhisicalValue();
        stepCount++;
      } else {
        a = x1;
        if ((b - a) <= FIB_MIN_INTERVAL) {
          break;
        }
        x1 = x2;
        A = B;
        x2 = b - FIB_INTERVAL_RATIO * (b - a);
        B = moveAndMeasure(actuator, round(x2));
        bp = actuator.getPhisicalValue();
        stepCount++;
      }
    }
    _LOGD("optimise", "Fib:[%d] Finished: Pa(%f)=%f, Pb(%f)=%f, %s->%f", stepCount, a, A, b, B, actuator.getName(), ap);
    return optimiseLinear(actuator, round(a), round(b), stepCount);
  }

  virtual float optimiseLinear(Actuators::Actuator &actuator, uint32_t a, uint32_t b, uint16_t &stepCount) {
    _LOGD("optimize", "Phase 3: Linear search of the minimum. a=%d, b=%d", a, b);
    stepCount++;
    float prevStepMeasurement = moveAndMeasure(actuator, a);
    do {
      a++;
      stepCount++;
      float currentStepMeasurement = moveAndMeasure(actuator, a);
      _LOGD("optimise", "Ln:[%d]: P(%d)=%f", stepCount, a, currentStepMeasurement);
      if (currentStepMeasurement > prevStepMeasurement) {
        a--;
        moveAndWait(actuator, a);
        break;
      }
      prevStepMeasurement = currentStepMeasurement;
    } while (a < b);
    _LOGD("optimise", "Ln:[%d] Finished: Pa(%d)=%f, %s->%f", stepCount, a, prevStepMeasurement, actuator.getName(), actuator.getPhisicalValue());
    return prevStepMeasurement;
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

  virtual void tuneCycle(){};

  virtual void tune() {
    tuneCycle();
    /*
    float target = measureAndWait();
    uint16_t cnt = 1;
    while (true) {
      tuneCycle();
      if (!_swrMeter.isInRange()) {
        break;
      }
      if (_swrMeter.getTarget() >= target) {
        break;
      }
      _LOGD("atu", "[%d] Tuning cycle P()=%f", cnt, target);
      cnt++;
    }
    */
  };

  virtual void onCommand(Core::command_type_t type, const JsonObject &doc) override {
    if (type == Core::COMMAND_TYPE_TUNE) {
      static bool lock = false;
      if (lock) {
        _LOGE("atu", "Core::COMMAND_TYPE_TUNE: Command execution in progress, aborting.");
        return;
      }
      lock = true;
      if (!doc["config"].isNull()) {
        setGlobalConfig(doc["config"].as<JsonObject>());
      }
      _LOGD("atu", "Core::COMMAND_TYPE_TUNE: waiting for isReady().");
      busyWait([this]() { return !isReady(); });
      turnOnTrx();
      _waitTickTimeout = 10;
      _LOGD("atu", "Core::COMMAND_TYPE_TUNE: waiting for swrMeter.isInRange().");
      busyWait([this]() { return !(_swrMeter.isInRange() || (_waitTickTimeout == 0)); });
      if (_swrMeter.isInRange()) {
        tune();
      } else {
        _LOGE("atu", "Core::COMMAND_TYPE_TUNE: Timeout waiting for swrMeter.isInRange(), aborting.");
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