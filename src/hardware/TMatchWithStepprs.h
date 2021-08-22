#pragma once

#include "FastAccelStepper.h"
#include "SWRMeterAds1115Ad8310.h"
#include "actuators/ActuatorStepper.h"
#include "core/Component.h"
#include "debug.h"
#include "device/TMatch.h"
#include <Arduino.h>

/*
  Board pin definitions
*/
#define L_DRIVER_PIN_STEP 17
#define L_DRIVER_PIN_DIR 16
#define L_SENSOR_PIN 32

#define C1_DRIVER_PIN_STEP 4
#define C1_DRIVER_PIN_DIR 2
#define C1_SENSOR_PIN 35

#define C2_DRIVER_PIN_STEP 18
#define C2_DRIVER_PIN_DIR 19
#define C2_SENSOR_PIN 34

#define ALL_MOTORS_ENABLE_PIN 5
#define ADS1115_ALERT_READY_PIN 23

/*
  Mechanical parameters
*/
#define L_RANGE_IN_STEPS 2964 * 14
#define L_SPEED 5500
#define L_ACCELERATION 4000

#define C1_RANGE_IN_STEPS 2964
#define C1_SPEED 1000
#define C1_ACCELERATION 2000

#define C2_RANGE_IN_STEPS 2964
#define C2_SPEED 1000
#define C2_ACCELERATION 2000

/*
  System parameters
*/
#define L_ACTUATOR_NAME "L"
#define C1_ACTUATOR_NAME "C1"
#define C2_ACTUATOR_NAME "C2"

#define L_ACTUATOR_MIN 0.05
#define L_ACTUATOR_MAX 10.7

#define C1_ACTUATOR_MIN 150.0
#define C1_ACTUATOR_MAX 150.0 + 360.0

#define C2_ACTUATOR_MIN 150.0
#define C2_ACTUATOR_MAX 150.0 + 360.0

#define HW_INFO "AD8310/3xSteppers"

namespace Hardware {

using namespace Sensor;
using namespace Core;
using namespace Actuators;
using namespace Device;

static SWRMeterAds1115Ad8310 swr(ADS1115_ALERT_READY_PIN);

FastAccelStepperEngine engine = FastAccelStepperEngine();

FastAccelStepper *stepperL = NULL;
FastAccelStepper *stepperC1 = NULL;
FastAccelStepper *stepperC2 = NULL;

ActuatorStepperPowerManager pwrManager(ALL_MOTORS_ENABLE_PIN);
ActuatorStepper actuatorL(stepperL, L_SENSOR_PIN, L_RANGE_IN_STEPS, L_ACTUATOR_NAME, L_ACTUATOR_MIN, L_ACTUATOR_MAX);
ActuatorStepper actuatorC1(stepperC1, C1_SENSOR_PIN, C1_RANGE_IN_STEPS, C1_ACTUATOR_NAME, C1_ACTUATOR_MIN, C1_ACTUATOR_MAX);
ActuatorStepper actuatorC2(stepperC2, C2_SENSOR_PIN, C2_RANGE_IN_STEPS, C2_ACTUATOR_NAME, C2_ACTUATOR_MIN, C2_ACTUATOR_MAX);

class TMatchWithSteppers : public Device::TMatch {
public:
  TMatchWithSteppers() : Device::TMatch(swr, actuatorL, actuatorC1, actuatorC2) {
    _actuatorLInitial = 3000;
    _actuatorC1Initial = 120;
    _actuatorC2Initial = 120;
    _actuatorLInitialStep = 10;
    _actuatorC1InitialStep = 10;
    _actuatorC2InitialStep = 10;
  }

  virtual void init() override {
    pinMode(ALL_MOTORS_ENABLE_PIN, OUTPUT);
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

    actuatorL.registerPowerManager(pwrManager);
    actuatorC1.registerPowerManager(pwrManager);
    actuatorC2.registerPowerManager(pwrManager);

    TMatch::init();
  };

  virtual void timer250() override {
    TMatch::timer250();
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  };
};

TMatchWithSteppers atu;

} // namespace Hardware
