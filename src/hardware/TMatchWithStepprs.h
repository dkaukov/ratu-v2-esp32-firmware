#pragma once

#include "SWRMeterAds1115Ad8310.h"
#include "actuators/ActuatorStepper.h"
#include "core/Component.h"
#include "debug.h"
#include "device/ATU.h"
#include <AccelStepper.h>
#include <Arduino.h>

#define I2C_ADDRESS 0x48

namespace Hardware {

using namespace Sensor;
using namespace Core;
using namespace Actuators;

static SWRMeterAds1115Ad8310 swr(COMPONENT_CLASS_SENSOR, I2C_ADDRESS, 23);
AccelStepper stepperL = AccelStepper(1, 19, 18);  // Custom pinout "L" - Step to GPI019, Dir to GPI018 (Default AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5)
AccelStepper stepperC1 = AccelStepper(1, 17, 16); // Custom pinout "C1" - Step to GPI017, Dir to GPI016 (Default AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5)
AccelStepper stepperC2 = AccelStepper(1, 0, 4);   // Custom pinout "C2" - Step to GPI00, Dir to GPI04 (Default AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5)

ActuatorStepperPowerManager pwrManager(5);
ActuatorStepper actuatorL(stepperL, 26, 32000, "L");
ActuatorStepper actuatorC1(stepperC1, 14, 32000, "C1");
ActuatorStepper actuatorC2(stepperC2, 27, 32000, "C2");

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
    stepperL.setMaxSpeed(5500);     //  Set maximum roration speed for "L" Motor 1
    stepperL.setSpeed(5500);        //  Set maximum calibration speed for "L" Motor 1
    stepperL.setAcceleration(4000); //  Set maximum acceleration for "L" Motor 1

    stepperC1.setMaxSpeed(2000);     //  Set maximum roration speed for "C" Motor 2
    stepperC1.setSpeed(1000);        //  Set maximum calibration speed for "C" Motor 2
    stepperC1.setAcceleration(2000); //  Set maximum acceleration for "C" Motor 2

    stepperC2.setMaxSpeed(2000);     //  Set maximum roration speed for "C" Motor 3
    stepperC2.setSpeed(1000);        //  Set maximum calibration speed for "C" Motor 3
    stepperC2.setAcceleration(2000); //  Set maximum acceleration for "C" Motor 3

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
    uint32_t startedTime = micros();
    optimise(actuatorL, _actuatorLInitialStep, _historesis);
    _LOGI("autoTune", "stepperL finished in %8d ms", (uint32_t)micros() - startedTime);
    optimise(actuatorC1, _actuatorC1InitialStep, _historesis);
    _LOGI("autoTune", "stepperC1 finished in %8d ms", (uint32_t)micros() - startedTime);
    optimise(actuatorC2, _actuatorC2InitialStep, _historesis);
    _LOGI("autoTune", "stepperC2 finished in %8d ms", (uint32_t)micros() - startedTime);
  };

  virtual void timer1000() override {
    StaticJsonDocument<1024> doc;
    getGlobalStatus(doc);
    String output;
    serializeJson(doc, output);
    _LOGI(_name, "Status: %s", output.c_str());
  };
};

} // namespace Hardware