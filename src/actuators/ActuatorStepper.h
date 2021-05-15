#pragma once

#include "AccelStepper.h"
#include "Actuator.h"
#include "core/Component.h"
#include <Arduino.h>

namespace Actuators {

class ActuatorStepper : public Actuator {
protected:
  AccelStepper &_stepper;
  uint8_t _sensorPin;
  uint8_t _enablePin;
  bool _enabled = false;

  virtual void enable() {
    if (!_enabled) {
      digitalWrite(_enablePin, LOW);
      delay(5);
      _enabled = true;
    }
  };

  virtual void disable() {
    digitalWrite(_enablePin, HIGH);
    _enabled = false;
  };

  virtual void managePower() {
    static const unsigned long REFRESH_INTERVAL = 100; // ms
    static unsigned long lastRefreshTime = 0;
    if (millis() - lastRefreshTime >= REFRESH_INTERVAL) {
      lastRefreshTime = millis();
      if (!_stepper.isRunning()) {
        disable();
      }
    }
  }

public:
  ActuatorStepper(AccelStepper &stepper,
                  const uint8_t enablePin,
                  const uint8_t sensorPin,
                  const char *name) : Actuator(Core::COMPONENT_CLASS_ACTUATOR, name),
                                      _stepper(stepper),
                                      _sensorPin(sensorPin),
                                      _enablePin(enablePin){};

  virtual void init() {
    _max = 32000;
    pinMode(_sensorPin, INPUT);
    pinMode(_enablePin, OUTPUT);
    enable();
    calibrate();
  };

  virtual void loop() {
    if (!_stepper.run()) {
      managePower();
    }
  };

  virtual void calibrate(bool restoreState = false) {
    _LOGI("calibrate", "Calibration of %s started.", _name);
    int32_t oldPosition = _stepper.currentPosition();
    int32_t range = (_max - _min);
    int32_t stepsTraveled = 0;
    _stepper.setSpeed(-3500);
    while (digitalRead(_sensorPin) == LOW) {
      if (_stepper.runSpeed()) {
        stepsTraveled++;
      }
      if (stepsTraveled >= range) {
        _LOGE("calibrate", "Calibration of %s apborted, as end-stop was not riched in %d steps.", _name, stepsTraveled);
        disable();
        return;
      }
    }
    _LOGI("calibrate", "Calibration of %s finished in %d steps.", _name, stepsTraveled);
    _stepper.stop();
    _stepper.setCurrentPosition(0);
    if (restoreState) {
      _stepper.moveTo(oldPosition);
    }
  };

  virtual int32_t getValue() { return _stepper.currentPosition(); };

  virtual float getPhisicalValue() { return getValue(); };

  virtual int32_t setValue(int32_t pos) {
    int32_t oldPosition = _stepper.currentPosition();
    pos = constrain(pos, _min, _max);
    enable();
    _stepper.moveTo(pos);
    return pos - oldPosition;
  };

  virtual bool isReady() { return !_stepper.isRunning(); }
};

} // namespace Actuators
