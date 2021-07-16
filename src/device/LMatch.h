#pragma once

#include "actuators/Actuator.h"
#include "core/Component.h"
#include "device/ATU.h"
#include <Arduino.h>

namespace Device {

using namespace Sensor;
using namespace Core;
using namespace Actuators;

typedef enum {
  ATU_TUNING_TYPE_L,
  ATU_TUNING_TYPE_C,
} atu_tuning_type_t;

class LMatch : public Device::ATU {
protected:
  Actuators::Actuator &_actuatorL;
  Actuators::Actuator &_actuatorC1;

  int32_t _actuatorLInitial = 0;
  int32_t _actuatorC1Initial = 0;

  int32_t _actuatorLInitialStep = 0;
  int32_t _actuatorC1InitialStep = 0;

  float _historesis = 0.0;

  atu_tuning_type_t _tuningMode = ATU_TUNING_TYPE_L;

  atu_tuning_type_t stringToTuningType(const String &val) const {
    if (val == "ATU_TUNING_TYPE_L") {
      return ATU_TUNING_TYPE_L;
    }
    if (val == "ATU_TUNING_TYPE_C") {
      return ATU_TUNING_TYPE_C;
    }
    return ATU_TUNING_TYPE_L;
  }

  const char *tuningTypeToString(const atu_tuning_type_t val) const {
    switch (val) {
    case ATU_TUNING_TYPE_L:
      return "ATU_TUNING_TYPE_L";
    case ATU_TUNING_TYPE_C:
      return "ATU_TUNING_TYPE_C";
    default:
      return "";
    }
  }

public:
  LMatch(SWRMeter &swrMeter,
         Actuator &actuatorL,
         Actuator &actuatorC1) : Device::ATU(Core::COMPONENT_CLASS_GENERIC, swrMeter),
                                 _actuatorL(actuatorL),
                                 _actuatorC1(actuatorC1) {
  }

  virtual void init() override {
    Device::ATU::init();
    _swrMeter.init();
    _actuatorL.init();
    _actuatorC1.init();
    resetToDefaults();
  };

  virtual void tuneCycle() override {
    ATU::tuneCycle();
    uint32_t startedTime = millis();
    _LOGI("autoTune", "start: %s=%f(%d), %s=%f(%d)",
          _actuatorL.getName(), _actuatorL.getPhisicalValue(), _actuatorL.getValue(),
          _actuatorC1.getName(), _actuatorC1.getPhisicalValue(), _actuatorC1.getValue());

    if (_tuningMode == ATU_TUNING_TYPE_L) {
      optimise(_actuatorL, _actuatorLInitialStep, _historesis);
      _LOGI("autoTune", "%s finished in %8d ms", _actuatorL.getName(), (uint32_t)millis() - startedTime);
      optimise(_actuatorC1, _actuatorC1InitialStep, _historesis);
      _LOGI("autoTune", "%s finished in %8d ms", _actuatorC1.getName(), (uint32_t)millis() - startedTime);
    } else {
      optimise(_actuatorC1, _actuatorC1InitialStep, _historesis);
      _LOGI("autoTune", "%s finished in %8d ms", _actuatorC1.getName(), (uint32_t)millis() - startedTime);
      optimise(_actuatorL, _actuatorLInitialStep, _historesis);
      _LOGI("autoTune", "%s finished in %8d ms", _actuatorL.getName(), (uint32_t)millis() - startedTime);
    }

    _LOGI("autoTune", "finish: %s=%f(%d), %s=%f(%d)",
          _actuatorL.getName(), _actuatorL.getPhisicalValue(), _actuatorL.getValue(),
          _actuatorC1.getName(), _actuatorC1.getPhisicalValue(), _actuatorC1.getValue());
  };

  virtual void setConfig(const JsonObject &doc) override {
    ATU::setConfig(doc);
    auto node = doc["atu"];
    if (!node["L"]["initial"].isNull()) {
      _actuatorLInitial = node["L"]["initial"];
    }
    if (!node["C1"]["initial"].isNull()) {
      _actuatorC1Initial = node["C1"]["initial"];
    }
    if (!node["L"]["step"].isNull()) {
      _actuatorLInitialStep = node["L"]["step"];
    }
    if (!node["C1"]["step"].isNull()) {
      _actuatorC1InitialStep = node["C1"]["step"];
    }
    if (!node["tuningMode"].isNull()) {
      _tuningMode = stringToTuningType(node["tuningMode"]);
    }
  };

  virtual void resetToDefaults() override {
    _LOGI("resetToDefaults", "Moving actuators to the default positions(s): L=%d, C1=%d", _actuatorLInitial, _actuatorC1Initial);
    _actuatorL.setValue(_actuatorLInitial);
    _actuatorC1.setValue(_actuatorC1Initial);
  };

  virtual bool isReady() const override { return _actuatorL.isReady() && _actuatorC1.isReady(); }

  virtual void getStatus(JsonObject &doc) const override {
    ATU::getStatus(doc);
    auto node = doc["atu"];
    node["tuningMode"] = tuningTypeToString(_tuningMode);
  }
};

} // namespace Device
