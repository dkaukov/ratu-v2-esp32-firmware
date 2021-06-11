#pragma once

#include "actuators/Actuator.h"
#include "core/Component.h"
#include "device/ATU.h"
#include "device/LMatch.h"
#include <Arduino.h>

namespace Device {

using namespace Sensor;
using namespace Core;
using namespace Actuators;

class TMatch : public Device::LMatch {
protected:
  Actuator &_actuatorC2;

  int32_t _actuatorC2Initial = 0;
  int32_t _actuatorC2InitialStep = 0;

public:
  TMatch(SWRMeter &swrMeter,
         Actuator &actuatorL,
         Actuator &actuatorC1,
         Actuator &actuatorC2) : LMatch(swrMeter, actuatorL, actuatorC1),
                                 _actuatorC2(actuatorC2) {
  }

  virtual void init() override {
    _actuatorC2.init();
    LMatch::init();
  };

  virtual void tuneCycle() override {
    ATU::tuneCycle();
    uint32_t startedTime = millis();
    _LOGI("autoTune", "start: %s=%f(%d), %s=%f(%d), %s=%f(%d)",
          _actuatorL.getName(), _actuatorL.getPhisicalValue(), _actuatorL.getValue(),
          _actuatorC1.getName(), _actuatorC1.getPhisicalValue(), _actuatorC1.getValue(),
          _actuatorC2.getName(), _actuatorC2.getPhisicalValue(), _actuatorC2.getValue());
    optimise(_actuatorL, _actuatorLInitialStep, _historesis);
    _LOGD("autoTune", "actuatorL finished in %8d ms", (uint32_t)millis() - startedTime);
    optimise(_actuatorC1, _actuatorC1InitialStep, _historesis);
    _LOGD("autoTune", "actuatorC1 finished in %8d ms", (uint32_t)millis() - startedTime);
    optimise(_actuatorC2, _actuatorC2InitialStep, _historesis);
    _LOGD("autoTune", "actuatorC2 finished in %8d ms", (uint32_t)millis() - startedTime);
    _LOGI("autoTune", "finish: %s=%f(%d), %s=%f(%d), %s=%f(%d)",
          _actuatorL.getName(), _actuatorL.getPhisicalValue(), _actuatorL.getValue(),
          _actuatorC1.getName(), _actuatorC1.getPhisicalValue(), _actuatorC1.getValue(),
          _actuatorC2.getName(), _actuatorC2.getPhisicalValue(), _actuatorC2.getValue());
  };

  virtual void setConfig(const JsonObject &doc) override {
    LMatch::setConfig(doc);
    auto node = doc["atu"];
    if (!node["C2"]["initial"].isNull()) {
      _actuatorC2Initial = node["C2"]["initial"];
    }
    if (!node["C2"]["step"].isNull()) {
      _actuatorC2InitialStep = node["C2"]["step"];
    }
  };

  virtual void resetToDefaults() override {
    _LOGI("resetToDefaults", "Moving actuators to the default positions(s): L=%d, C1=%d, C2=%d", _actuatorLInitial, _actuatorC1Initial, _actuatorC2Initial);
    _actuatorL.setValue(_actuatorLInitial);
    _actuatorC1.setValue(_actuatorC1Initial);
    _actuatorC2.setValue(_actuatorC2Initial);
  };

  virtual bool isReady() const override { return _actuatorL.isReady() && _actuatorC1.isReady() && _actuatorC2.isReady(); }
};

} // namespace Device