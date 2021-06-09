#pragma once

#include "FastAccelStepper.h"
#include "SWRMeterAds1115Ad8310.h"
#include "actuators/ActuatorStepper.h"
#include "core/Component.h"
#include "debug.h"
#include "device/ATU.h"
#include <Arduino.h>

/*
  Board pin definitions
*/
#define L_DRIVER_PIN_STEP 19
#define L_DRIVER_PIN_DIR 18
#define L_SENSOR_PIN 26

#define C1_DRIVER_PIN_STEP 17
#define C1_DRIVER_PIN_DIR 16
#define C1_SENSOR_PIN 14

#define C2_DRIVER_PIN_STEP 0
#define C2_DRIVER_PIN_DIR 4
#define C2_SENSOR_PIN 27

#define ALL_MOTORS_ENABLE_PIN 5
#define ADS1115_ALERT_READY_PIN 23

/*
  Mechanical parameters
*/
#define L_RANGE_IN_STEPS 32000
#define L_SPEED 5500
#define L_ACCELERATION 4000

#define C1_RANGE_IN_STEPS 32000
#define C1_SPEED 1000
#define C1_ACCELERATION 2000

#define C2_RANGE_IN_STEPS 32000
#define C2_SPEED 1000
#define C2_ACCELERATION 2000

/*
  System parameters
*/
#define L_ACTUATOR_NAME "L"
#define C1_ACTUATOR_NAME "C1"
#define C2_ACTUATOR_NAME "C2"

namespace Hardware {

using namespace Sensor;
using namespace Core;
using namespace Actuators;

static SWRMeterAds1115Ad8310 swr(ADS1115_ALERT_READY_PIN);

FastAccelStepperEngine engine = FastAccelStepperEngine();

FastAccelStepper *stepperL = NULL;
FastAccelStepper *stepperC1 = NULL;
FastAccelStepper *stepperC2 = NULL;

ActuatorStepperPowerManager pwrManager(ALL_MOTORS_ENABLE_PIN);
ActuatorStepper actuatorL(stepperL, L_SENSOR_PIN, L_RANGE_IN_STEPS, L_ACTUATOR_NAME);
ActuatorStepper actuatorC1(stepperC1, C1_SENSOR_PIN, C1_RANGE_IN_STEPS, C1_ACTUATOR_NAME);
ActuatorStepper actuatorC2(stepperC2, C2_SENSOR_PIN, C2_RANGE_IN_STEPS, C2_ACTUATOR_NAME);

class TMatchWithSteppers : public Device::ATU {
private:
  int32_t _actuatorLInitial = 3000;
  int32_t _actuatorC1Initial = 120;
  int32_t _actuatorC2Initial = 120;

  int32_t _actuatorLInitialStep = 100;
  int32_t _actuatorC1InitialStep = 100;
  int32_t _actuatorC2InitialStep = 100;
  float _historesis = 0.0;

public:
  TMatchWithSteppers() : Device::ATU(COMPONENT_CLASS_GENERIC, swr) {}

  virtual void init() override {
    engine.init();
    stepperL = engine.stepperConnectToPin(L_DRIVER_PIN_STEP);
    if (stepperL) {
      stepperL->setDirectionPin(L_DRIVER_PIN_DIR);
      stepperL->setSpeedInHz(L_SPEED);
      stepperL->setAcceleration(L_ACCELERATION);
    }

    stepperC1 = engine.stepperConnectToPin(C1_DRIVER_PIN_STEP);
    if (stepperC1) {
      stepperC1->setDirectionPin(C1_DRIVER_PIN_DIR);
      stepperC1->setSpeedInHz(C1_SPEED);
      stepperC1->setAcceleration(C1_ACCELERATION);
    }

    stepperC2 = engine.stepperConnectToPin(C2_DRIVER_PIN_STEP);
    if (stepperC2) {
      stepperC2->setDirectionPin(C2_DRIVER_PIN_DIR);
      stepperC2->setSpeedInHz(C2_SPEED);
      stepperC2->setAcceleration(C2_ACCELERATION);
    }

    swr.init();
    actuatorL.registerPowerManager(pwrManager);
    actuatorC1.registerPowerManager(pwrManager);
    actuatorC2.registerPowerManager(pwrManager);
    actuatorL.init();
    actuatorC1.init();
    actuatorC2.init();
    resetToDefaults();
  };

  virtual void timer250() override {
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  };

  virtual void resetToDefaults() override {
    _LOGI("resetToDefaults", "Moving actuators to the default positions(s): L=%d, C1=%d, C2=%d", _actuatorLInitial, _actuatorC1Initial, _actuatorC2Initial);
    actuatorL.setValue(_actuatorLInitial);
    actuatorC1.setValue(_actuatorC1Initial);
    actuatorC2.setValue(_actuatorC2Initial);
  };

  virtual void tuneCycle() override {
    ATU::tuneCycle();
    uint32_t startedTime = millis();
    _LOGI("autoTune", "start: %s=%f(%d), %s=%f(%d), %s=%f(%d)",
          actuatorL.getName(), actuatorL.getPhisicalValue(), actuatorL.getValue(),
          actuatorC1.getName(), actuatorC1.getPhisicalValue(), actuatorC1.getValue(),
          actuatorC2.getName(), actuatorC2.getPhisicalValue(), actuatorC2.getValue());
    optimise(actuatorL, _actuatorLInitialStep, _historesis);
    _LOGI("autoTune", "stepperL finished in %8d ms", (uint32_t)millis() - startedTime);
    optimise(actuatorC1, _actuatorC1InitialStep, _historesis);
    _LOGI("autoTune", "stepperC1 finished in %8d ms", (uint32_t)millis() - startedTime);
    optimise(actuatorC2, _actuatorC2InitialStep, _historesis);
    _LOGI("autoTune", "stepperC2 finished in %8d ms", (uint32_t)millis() - startedTime);
    _LOGI("autoTune", "finish: %s=%f(%d), %s=%f(%d), %s=%f(%d)",
          actuatorL.getName(), actuatorL.getPhisicalValue(), actuatorL.getValue(),
          actuatorC1.getName(), actuatorC1.getPhisicalValue(), actuatorC1.getValue(),
          actuatorC2.getName(), actuatorC2.getPhisicalValue(), actuatorC2.getValue());
  };

  virtual bool isReady() const override { return actuatorL.isReady() && actuatorC1.isReady() && actuatorC2.isReady(); }

  virtual void setConfig(const JsonObject &doc) override {
    Device::ATU::setConfig(doc);
    auto node = doc["atu"];
    if (!node["L"]["initial"].isNull()) {
      _actuatorLInitial = node["L"]["initial"];
    }
    if (!node["C1"]["initial"].isNull()) {
      _actuatorC1Initial = node["C1"]["initial"];
    }
    if (!node["C2"]["initial"].isNull()) {
      _actuatorC2Initial = node["C2"]["initial"];
    }
    if (!node["L"]["step"].isNull()) {
      _actuatorLInitialStep = node["L"]["step"];
    }
    if (!node["C1"]["step"].isNull()) {
      _actuatorC1InitialStep = node["C1"]["step"];
    }
    if (!node["C2"]["step"].isNull()) {
      _actuatorC2InitialStep = node["C2"]["step"];
    }
  };
  
};

TMatchWithSteppers atu;

} // namespace Hardware