#pragma once

#include "SWRMeterAds1115Ad8310.h"
#include "actuators/ActuatorStepper.h"
#include "debug.h"
#include "device/ATU.h"
#include <AccelStepper.h>
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

AccelStepper stepperL = AccelStepper(AccelStepper::DRIVER, L_DRIVER_PIN_STEP, L_DRIVER_PIN_DIR);
AccelStepper stepperC1 = AccelStepper(AccelStepper::DRIVER, C1_DRIVER_PIN_STEP, C1_DRIVER_PIN_DIR);
AccelStepper stepperC2 = AccelStepper(AccelStepper::DRIVER, C2_DRIVER_PIN_STEP, C2_DRIVER_PIN_DIR);

ActuatorStepperPowerManager pwrManager(ALL_MOTORS_ENABLE_PIN);
ActuatorStepper actuatorL(stepperL, L_SENSOR_PIN, L_RANGE_IN_STEPS, L_ACTUATOR_NAME);
ActuatorStepper actuatorC1(stepperC1, C1_SENSOR_PIN, C1_RANGE_IN_STEPS, C1_ACTUATOR_NAME);
ActuatorStepper actuatorC2(stepperC2, C2_SENSOR_PIN, C2_RANGE_IN_STEPS, C2_ACTUATOR_NAME);

class TMatchWithSteppers : public Device::ATU {
private:
  int32_t _actuatorLInitial = 3000;
  int32_t _actuatorC1Initial = 120;
  int32_t _actuatorC2Initial = 120;

  int32_t _actuatorLInitialStep = 3000;
  int32_t _actuatorC1InitialStep = 100;
  int32_t _actuatorC2InitialStep = 100;
  float _historesis = 0.0;

public:
  TMatchWithSteppers() : Device::ATU(COMPONENT_CLASS_GENERIC, swr) {}

  virtual void init() override {
    stepperL.setMaxSpeed(L_SPEED);            //  Set maximum rotation speed for "L" Motor 1
    stepperL.setSpeed(L_SPEED);               //  Set maximum calibration speed for "L" Motor 1
    stepperL.setAcceleration(L_ACCELERATION); //  Set maximum acceleration for "L" Motor 1

    stepperC1.setMaxSpeed(C1_SPEED);            //  Set maximum rotation speed for "C1" Motor 2
    stepperC1.setSpeed(C1_SPEED);               //  Set maximum calibration speed for "C1" Motor 2
    stepperC1.setAcceleration(C1_ACCELERATION); //  Set maximum acceleration for "C1" Motor 2

    stepperC2.setMaxSpeed(C2_SPEED);            //  Set maximum rotation speed for "C2" Motor 3
    stepperC2.setSpeed(C2_SPEED);               //  Set maximum calibration speed for "C2" Motor 3
    stepperC2.setAcceleration(C2_ACCELERATION); //  Set maximum acceleration for "C2" Motor 3

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

  virtual void tune() override {
    ATU::tune();
    uint32_t startedTime = micros();
    optimise(actuatorL, _actuatorLInitialStep, _historesis);
    _LOGI("autoTune", "stepperL finished in %8d ms", (uint32_t)micros() - startedTime);
    optimise(actuatorC1, _actuatorC1InitialStep, _historesis);
    _LOGI("autoTune", "stepperC1 finished in %8d ms", (uint32_t)micros() - startedTime);
    optimise(actuatorC2, _actuatorC2InitialStep, _historesis);
    _LOGI("autoTune", "stepperC2 finished in %8d ms", (uint32_t)micros() - startedTime);
  };

  virtual bool isReady() const override { return actuatorL.isReady() && actuatorC1.isReady() && actuatorC2.isReady(); }
};

TMatchWithSteppers atu;

} // namespace Hardware