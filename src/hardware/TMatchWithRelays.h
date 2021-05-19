#pragma once

#include "SWRMeterAds1115Ad8310.h"
#include "actuators/ActuatorDACGPIO.h"
#include "actuators/ActuatorDACMCP23017.h"
#include "debug.h"
#include "device/ATU.h"
#include <Arduino.h>

/*
  Board pin definitions
*/
// ****** L network  MCP23017 GPA ******
#define K1 0 // 0.05 uH    1        // GPA7
#define K2 1 // 0.1 uH     2        // GPA6
#define K3 2 // 0.2 uH     4        // GPA5
#define K4 3 // 0.4 uH     8        // GPA4
#define K5 4 // 0.8 uH     16       // GPA3
#define K6 5 // 1.6 uH     32       // GPA2
#define K7 6 // 3.2 uH     64       // GPA1
#define K8 7 // 6.4 uH     128      // GPA0

// ****** C1 network  MCP23017 GPB  ******
#define K9 15  // 5 pF       1       // GPB7
#define K10 14 // 10 pF      2       // GPB6
#define K11 13 // 22 pF      4      // GPB5
#define K12 12 // 39 pF      8      // GPB4
#define K13 11 // 78 pF      16     // GPB3
#define K14 10 // 160 pF     32     // GPB2
#define K15 9  // 330 pF     64     // GPB1
#define K16 8  // 660 pF     128    // GPB0

#define K26 26 // bypass C1         // ESP32 GPI026

// ****** C2 network ESP32 GPIO ******
// PCB version 4
#define K17 18 // 5 pF        1    // ESP32 GPI018
#define K18 5  // 10 pF       2    // ESP32 GPI05
#define K19 17 // 22 pF       4    // ESP32 GPI017
#define K20 16 // 39 pF       8    // ESP32 GPI016
#define K21 4  // 78 pF       16   // ESP32 GPI04
#define K22 2  // 160 pF      32   // ESP32 GPI02
#define K23 15 // 330 pF      64   // ESP32 GPI015
#define K24 23 // 660 pF      128  // ESP32 GPI023

#define K25 27 // bypass C2         // ESP32 GPI027

#define ADS1115_ALERT_READY_PIN 19

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

static Adafruit_MCP23017 mcp;
ActuatorDACMCP23017 actuatorL(L_ACTUATOR_NAME, {pin : {K1, K2, K3, K4, K5, K6, K7, K8}}, mcp);
ActuatorDACMCP23017 actuatorC1(C1_ACTUATOR_NAME, {pin : {K9, K10, K11, K12, K13, K14, K15, K16}}, mcp);
ActuatorDACGPIO actuatorC2(C2_ACTUATOR_NAME, {pin : {K17, K18, K19, K20, K21, K22, K23, K24}});

typedef enum {
  ATU_MODE_TMATCH,
  ATU_MODE_LC,
  ATU_MODE_CL,
} atu_mode_type_t;

class TMatchWithRelays : public Device::ATU {
private:
  atu_mode_type_t _mode = ATU_MODE_TMATCH;

  int32_t _actuatorLInitial = 8;
  int32_t _actuatorC1Initial = 64;
  int32_t _actuatorC2Initial = 64;

  int32_t _actuatorLInitialStep = 2;
  int32_t _actuatorC1InitialStep = -8;
  int32_t _actuatorC2InitialStep = -8;
  float _historesis = 0.0;

public:
  TMatchWithRelays() : Device::ATU(COMPONENT_CLASS_GENERIC, swr) {
  }

  virtual void init() override {
    mcp.begin();
    pinMode(K25, OUTPUT);
    pinMode(K26, OUTPUT);
    swr.init();
    actuatorL.init();
    actuatorC1.init();
    actuatorC2.init();
    resetToDefaults();
  };

  virtual void resetToDefaults() override {
    _LOGI("resetToDefaults", "Moving actuators to the default positions(s): L=%d, C1=%d, C2=%d", _actuatorLInitial, _actuatorC1Initial, _actuatorC2Initial);
    actuatorL.setValue(_actuatorLInitial);
    actuatorC1.setValue(_actuatorC1Initial);
    actuatorC2.setValue(_actuatorC2Initial);
  };

  virtual void setMode(atu_mode_type_t mode) {
    if (mode != _mode) {
      switch (mode) {
      case ATU_MODE_TMATCH:
        digitalWrite(K25, LOW);
        digitalWrite(K26, LOW);
        break;
      case ATU_MODE_LC:
        digitalWrite(K25, HIGH);
        digitalWrite(K26, LOW);
        break;
      case ATU_MODE_CL:
        digitalWrite(K25, LOW);
        digitalWrite(K26, HIGH);
        break;
      }
      _mode = mode;
    }
  }

  virtual void tune() override {
    uint32_t startedTime = micros();
    if (_mode == ATU_MODE_TMATCH) {
      optimise(actuatorL, _actuatorLInitialStep, _historesis);
      _LOGI("autoTune", "actuatorL finished in %8d ms", (uint32_t)micros() - startedTime);
      optimise(actuatorC1, _actuatorC1InitialStep, _historesis);
      _LOGI("autoTune", "actuatorC1 finished in %8d ms", (uint32_t)micros() - startedTime);
      optimise(actuatorC2, _actuatorC2InitialStep, _historesis);
      _LOGI("autoTune", "actuatorC2 finished in %8d ms", (uint32_t)micros() - startedTime);
    }
    if (_mode == ATU_MODE_CL) {
      optimise(actuatorC1, _actuatorC1InitialStep, _historesis);
      _LOGI("autoTune", "actuatorC1 finished in %8d ms", (uint32_t)micros() - startedTime);
      optimise(actuatorL, _actuatorLInitialStep, _historesis);
      _LOGI("autoTune", "actuatorL finished in %8d ms", (uint32_t)micros() - startedTime);
    }
    if (_mode == ATU_MODE_LC) {
      optimise(actuatorC2, _actuatorC2InitialStep, _historesis);
      _LOGI("autoTune", "actuatorC2 finished in %8d ms", (uint32_t)micros() - startedTime);
      optimise(actuatorL, _actuatorLInitialStep, _historesis);
      _LOGI("autoTune", "actuatorL finished in %8d ms", (uint32_t)micros() - startedTime);
    }
  };

  virtual void setConfig(const JsonObject &doc) override {
    Device::ATU::setConfig(doc);
    auto node = doc["atu"];
    if (!node["mode"].isNull()) {
      atu_mode_type_t mode = node["mode"];
      if (mode != _mode) {
        setMode(node["mode"]);
        _LOGI("atu", "Setting ATU mode: %d", _mode);
      }
    }
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

  virtual void getStatus(JsonObject &doc) const override {
    Device::ATU::getStatus(doc);
    auto node = doc["atu"];
    node["mode"] = _mode;
  }
};

} // namespace Hardware