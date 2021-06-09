#pragma once

#include "Actuator.h"
#include "FastAccelStepper.h"
#include "core/Component.h"
#include "etl/observer.h"
#include <Arduino.h>

namespace Actuators {

struct PowerRequest {
};

static PowerRequest powerRequest = {};

typedef etl::observer<PowerRequest> PowerRequest_Observer;

class ActuatorStepperPowerManager : public PowerRequest_Observer, Core::Component {
private:
  uint8_t _enablePin;
  bool _enabled = false;
  uint8_t _enabledTicks;
  uint8_t _idleTimeoutTicks = 4;

public:
  ActuatorStepperPowerManager(uint8_t enablePin) : Core::Component(Core::COMPONENT_CLASS_POWER_MANAGER), _enablePin(enablePin){};

  virtual void enable() {
    if (!_enabled) {
      _LOGD("pwr", "Enabling stepper(s) power using pin %d", _enablePin);
      digitalWrite(_enablePin, LOW);
      delay(5);
      _enabled = true;
    }
    _enabledTicks = _idleTimeoutTicks;
  };

  virtual void disable() {
    if (_enabled) {
      _LOGD("pwr", "Disabling stepper(s) power using pin %d", _enablePin);
      digitalWrite(_enablePin, HIGH);
      _enabled = false;
    }
  };

  virtual void timer250() {
    if (_enabledTicks > 0) {
      _enabledTicks--;
    } else {
      if (_enabled) {
        disable();
      }
    }
  };

  virtual void init() {
    pinMode(_enablePin, OUTPUT);
  };

  void notification(PowerRequest button) { enable(); };

  virtual void getStatus(JsonObject &doc) const override {
    doc["pwr"]["pin"] = _enablePin;
    doc["pwr"]["enabled"] = _enabled;
    doc["pwr"]["timeout"] = _idleTimeoutTicks;
  };

  virtual void setConfig(const JsonObject &doc) override {
    if (!doc["pwr"]["timeout"].isNull()) {
      _idleTimeoutTicks = doc["pwr"]["timeout"];
    }
  };
};

class ActuatorStepper : public Actuator,
                        etl::observable<PowerRequest_Observer, 1> {
protected:
  FastAccelStepper *&_stepper;
  uint8_t _sensorPin;
  bool _enabled = false;
  int32_t _targetPosition;

  virtual void
  enable() {
    notify_observers(powerRequest);
  };

public:
  ActuatorStepper(FastAccelStepper *&stepper,
                  const uint8_t sensorPin,
                  const uint32_t max,
                  const char *name) : Actuator(Core::COMPONENT_CLASS_ACTUATOR, name),
                                      _stepper(stepper),
                                      _sensorPin(sensorPin) {
    _max = max;
  };

  virtual void init() override {
    pinMode(_sensorPin, INPUT);
    enable();
    calibrate();
  };

  virtual void loop() override {
    if (_stepper->isRunning()) {
      enable();
    }
  };

  virtual void calibrate(bool restoreState = false) override {
    _LOGI("calibrate", "Calibration of %s started.", _name);
    int32_t oldPosition = getTargetValue();
    int32_t range = (_max - _min);
    int32_t stepsTraveled = 0;
    uint32_t oldSpeedInUs = _stepper->getSpeedInUs();
    _stepper->setSpeedInHz(3500);
    _stepper->runBackward();
    while (digitalRead(_sensorPin) == LOW) {
      stepsTraveled = (oldPosition - _stepper->getCurrentPosition());
      if (stepsTraveled >= range) {
        _LOGE("calibrate", "Calibration of %s aborted, as end-stop was not riched in %d steps.", _name, stepsTraveled);
        _stepper->forceStopAndNewPosition(0);
        _stepper->setSpeedInUs(oldSpeedInUs);
        _targetPosition = 0;
        return;
      }
    }
    _LOGI("calibrate", "Calibration of %s finished in %d steps.", _name, stepsTraveled);
    _stepper->forceStopAndNewPosition(0);
    _stepper->setSpeedInUs(oldSpeedInUs);
    _targetPosition = 0;
    if (restoreState) {
      setValue(oldPosition);
    }
  };

  virtual int32_t getValue() const override { return _stepper->getCurrentPosition(); };
  virtual int32_t getTargetValue() const override { return _targetPosition; };

  virtual int32_t setValue(int32_t pos) override {
    int32_t oldPosition = _targetPosition;
    pos = constrain(pos, _min, _max);
    enable();
    _stepper->moveTo(pos);
    _targetPosition = pos;
    return pos - oldPosition;
  };

  virtual bool isReady() const override { return !_stepper->isRunning(); }

  virtual void registerPowerManager(ActuatorStepperPowerManager &pwrManager) {
    add_observer(pwrManager);
  }
};

} // namespace Actuators
