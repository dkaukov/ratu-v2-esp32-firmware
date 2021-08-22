#pragma once

#include "SWRMeterAds1115Ad8310.h"
#include "actuators/ActuatorDACGPIO.h"
#include "actuators/ActuatorDACMCP23017.h"
#include "debug.h"
#include "device/TMatch.h"
#include <Arduino.h>

//HW PCB version 0.6 (current)
/*
  Board pin definitions
*/
// ****** L network  MCP23017 GPA ******
#define K1 0 // 0.05 uH    1        // GPA0
#define K2 1 // 0.1 uH     2        // GPA1
#define K3 2 // 0.2 uH     4        // GPA2
#define K4 3 // 0.4 uH     8        // GPA3
#define K5 4 // 0.8 uH     16       // GPA4
#define K6 5 // 1.6 uH     32       // GPA5
#define K7 6 // 3.2 uH     64       // GPA6
#define K8 7 // 6.4 uH     128      // GPA7

// ****** C1 network  MCP23017 GPB  ******
#define K9 15  // 5 pF       1       // GPB7
#define K10 14 // 10 pF      2       // GPB6
#define K11 13 // 20 pF      4       // GPB5
#define K12 12 // 40 pF      8       // GPB4
#define K13 11 // 80 pF      16      // GPB3
#define K14 10 // 160 pF     32      // GPB2
#define K15 9  // 320 pF     64      // GPB1
#define K16 8  // 640 pF     128     // GPB0

#define K26 25 // bypass C1         // ESP32 GPI025 HW ver 0.6
//#define K26 26 // bypass C1         // ESP32 GPI026 HW ver 0.4

// ****** C2 network ESP32 GPIO ******
#define K17 14 // 5 pF        1    // ESP32 GPI014
#define K18 12 // 10 pF       2    // ESP32 GPI012
#define K19 26 // 20 pF       4    // ESP32 GPI026
#define K20 19 // 40 pF       8    // ESP32 GPI019
#define K21 17 // 80 pF       16   // ESP32 GPI017
#define K22 16 // 160 pF      32   // ESP32 GPI016
#define K23 15 // 320 pF      64   // ESP32 GPI15
#define K24 18 // 640 pF      128  // ESP32 GPI018

#define K25 27 // bypass C2         // ESP32 GPI027

//HW PCB version 0.5
// ****** C2 network ESP32 GPIO ******
/*
#define K17 18 // 5 pF        1    // ESP32 GPI018
#define K18 5  // 10 pF       2    // ESP32 GPI05
#define K19 17 // 22 pF       4    // ESP32 GPI017
#define K20 16 // 39 pF       8    // ESP32 GPI016
#define K21 4  // 78 pF       16   // ESP32 GPI04
#define K22 2  // 160 pF      32   // ESP32 GPI02
#define K23 15 // 330 pF      64   // ESP32 GPI015
#define K24 23 // 660 pF      128  // ESP32 GPI023

#define K25 27 // bypass C2         // ESP32 GPI027
*/

#define ADS1115_ALERT_READY_PIN 23

/*
  System parameters
*/
#define L_ACTUATOR_NAME "L"
#define C1_ACTUATOR_NAME "C1"
#define C2_ACTUATOR_NAME "C2"

#define L_ACTUATOR_MIN 0.05
#define L_ACTUATOR_MAX 12.75

#define C1_ACTUATOR_MIN 5.0
#define C1_ACTUATOR_MAX 1304.0

#define C2_ACTUATOR_MIN 5.0
#define C2_ACTUATOR_MAX 1304.0

#define HW_INFO "AD8310/8x8x8"

namespace Hardware {

using namespace Sensor;
using namespace Core;
using namespace Actuators;
using namespace Device;

static SWRMeterAds1115Ad8310 swr(ADS1115_ALERT_READY_PIN);

static Adafruit_MCP23017 mcp;
ActuatorDACMCP23017 actuatorL(L_ACTUATOR_NAME, {pin : {K1, K2, K3, K4, K5, K6, K7, K8}}, mcp, L_ACTUATOR_MIN, L_ACTUATOR_MAX);
ActuatorDACMCP23017 actuatorC1(C1_ACTUATOR_NAME, {pin : {K9, K10, K11, K12, K13, K14, K15, K16}}, mcp, C1_ACTUATOR_MIN, C1_ACTUATOR_MAX);
ActuatorDACGPIO actuatorC2(C2_ACTUATOR_NAME, {pin : {K17, K18, K19, K20, K21, K22, K23, K24}}, C2_ACTUATOR_MIN, C2_ACTUATOR_MAX);

typedef enum {
  ATU_MODE_TMATCH,
  ATU_MODE_LC,
  ATU_MODE_CL,
} atu_mode_type_t;

class TMatchWithRelays : public Device::TMatch {
protected:
  atu_mode_type_t _mode = ATU_MODE_TMATCH;

public:
  TMatchWithRelays() : Device::TMatch(swr, actuatorL, actuatorC1, actuatorC2) {
    _actuatorLInitial = 8;
    _actuatorC1Initial = 64;
    _actuatorC2Initial = 64;
    _actuatorLInitialStep = 4;
    _actuatorC1InitialStep = 4;
    _actuatorC2InitialStep = 4;
  }

  virtual void init() override {
    mcp.begin();
    pinMode(K25, OUTPUT);
    pinMode(K26, OUTPUT);
    TMatch::init();
  };

  virtual void setMode(atu_mode_type_t mode) {
    if (mode != _mode) {
      switch (mode) {
      case ATU_MODE_TMATCH:
        digitalWrite(K25, LOW);
        digitalWrite(K26, LOW);
        break;
      case ATU_MODE_CL:
        digitalWrite(K25, HIGH);
        digitalWrite(K26, LOW);
        break;
      case ATU_MODE_LC:
        digitalWrite(K25, LOW);
        digitalWrite(K26, HIGH);
        break;
      }
      _mode = mode;
    }
  }

  virtual void tuneCycle() override {
    if (_mode == ATU_MODE_TMATCH) {
      TMatch::tuneCycle();
      return;
    }
    if (_mode == ATU_MODE_CL) {
      LMatch::tuneCycle();
      return;
    }
    uint32_t startedTime = millis();
    _LOGI("autoTune", "start: %s=%f(%d), %s=%f(%d), %s=%f(%d)",
          actuatorL.getName(), actuatorL.getPhisicalValue(), actuatorL.getValue(),
          actuatorC1.getName(), actuatorC1.getPhisicalValue(), actuatorC1.getValue(),
          actuatorC2.getName(), actuatorC2.getPhisicalValue(), actuatorC2.getValue());

    if (_mode == ATU_MODE_LC) {
      if (_tuningMode == ATU_TUNING_TYPE_L) {
        optimise(actuatorL, _actuatorLInitialStep, _historesis);
        _LOGI("autoTune", "actuatorL finished in %8d ms", (uint32_t)millis() - startedTime);
        optimise(actuatorC2, _actuatorC2InitialStep, _historesis);
        _LOGI("autoTune", "actuatorC2 finished in %8d ms", (uint32_t)millis() - startedTime);
      } else {
        optimise(actuatorC2, _actuatorC2InitialStep, _historesis);
        _LOGI("autoTune", "actuatorC2 finished in %8d ms", (uint32_t)millis() - startedTime);
        optimise(actuatorL, _actuatorLInitialStep, _historesis);
        _LOGI("autoTune", "actuatorL finished in %8d ms", (uint32_t)millis() - startedTime);
      }
    }

    _LOGI("autoTune", "finish: %s=%f(%d), %s=%f(%d), %s=%f(%d)",
          actuatorL.getName(), actuatorL.getPhisicalValue(), actuatorL.getValue(),
          actuatorC1.getName(), actuatorC1.getPhisicalValue(), actuatorC1.getValue(),
          actuatorC2.getName(), actuatorC2.getPhisicalValue(), actuatorC2.getValue());
  };

  virtual void setConfig(const JsonObject &doc) override {
    TMatch::setConfig(doc);
    auto node = doc["atu"];
    if (!node["mode"].isNull()) {
      atu_mode_type_t mode = node["mode"];
      if (mode != _mode) {
        setMode(node["mode"]);
        _LOGI("atu", "Setting ATU mode: %d", _mode);
      }
    }
  };

  virtual void getStatus(JsonObject &doc) const override {
    TMatch::getStatus(doc);
    auto node = doc["atu"];
    node["mode"] = _mode;
  }

  virtual void getInfo(JsonObject &doc) const override {
    TMatch::getInfo(doc);
    auto node = doc["atu"];
    node["modes"][0] = "ATU_MODE_TMATCH";
    node["modes"][1] = "ATU_MODE_LC";
    node["modes"][2] = "ATU_MODE_CL";
  }
};

TMatchWithRelays atu;

} // namespace Hardware
