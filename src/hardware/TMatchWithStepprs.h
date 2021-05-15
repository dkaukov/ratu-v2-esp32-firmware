#pragma once

#include "SWRMeterAd8307.h"
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

static SWRMeterAd8307 swr(COMPONENT_CLASS_SENSOR, I2C_ADDRESS, 23);
AccelStepper stepperL = AccelStepper(1, 19, 18);  // Custom pinout "L" - Step to GPI019, Dir to GPI018 (Default AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5)
AccelStepper stepperC1 = AccelStepper(1, 17, 16); // Custom pinout "C1" - Step to GPI017, Dir to GPI016 (Default AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5)
AccelStepper stepperC2 = AccelStepper(1, 0, 4);   // Custom pinout "C2" - Step to GPI00, Dir to GPI04 (Default AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5)

ActuatorStepper actuatorL(stepperL, 5, 26, "L");
ActuatorStepper actuatorC1(stepperC1, 5, 14, "C1");
ActuatorStepper actuatorC2(stepperC2, 5, 27, "C2");

class TMatchWithSteppers : public Device::ATU {
private:
public:
  TMatchWithSteppers() : Device::ATU(COMPONENT_CLASS_GENERIC) {
  }

  virtual void init() {
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
    actuatorL.init();
    actuatorC1.init();
    actuatorC2.init();
  };
};

} // namespace Hardware