#pragma once

#include "core/Component.h"
#include "debug.h"
#include "sensor/SWRMeter.h"
#include "sensor/Sensor.h"
#include <ADS1115_WE.h>
#include <Arduino.h>
#include <Wire.h>

namespace Sensor {
    class SWRMeterSim : public SWRMeter {
    };
}