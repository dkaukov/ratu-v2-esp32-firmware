#pragma once

#include "core/Component.h"
#include "debug.h"
#include "sensor/SWRMeter.h"
#include "sensor/Sensor.h"
#include <ADS1115_WE.h>
#include <Arduino.h>
#include <Wire.h>

namespace Sensor {

#define ADS1115_DEFAULT_I2C_ADDRESS 0x48

class SWRMeterAds1115Ad8310 : public SWRMeter {

private:
  ADS1115_WE &_adc;
  const uint8_t _alertReadyPin;
  ADS1115_MUX _adcChannel = ADS1115_COMP_0_GND;
  uint32_t _conversionnCount = 0;
  float _fwdRaw;
  float _rflRaw;
  float _fwdIntercept = 840;
  float _rflIntercept = 840;

  ADS1115_WE &getAdc(int addr) {
    static ADS1115_WE __adc(addr);
    return __adc;
  }

public:
  SWRMeterAds1115Ad8310(uint8_t alertReadyPin,
                        uint8_t ads1115Addr = ADS1115_DEFAULT_I2C_ADDRESS)
      : SWRMeter(Core::COMPONENT_CLASS_SENSOR, "SWRMeterAds1115Ad8310"),
        _adc(getAdc(ads1115Addr)),
        _alertReadyPin(alertReadyPin){};

  void start() {
    _adc.setCompareChannels(_adcChannel);
    _adc.clearAlert();
    _adc.startSingleMeasurement();
  }

  float convertFwd(const float x) const { return exp((x - 1913.417895) / 102.743582); }

  float convertRfl(const float x) const { return exp((x - 1913.151197) / 101.409361); }

  virtual bool isInRange() const {
    return (_fwdRaw > _fwdIntercept) && (_rflRaw > _rflIntercept);
  }

  void read() {
    if (_adcChannel == ADS1115_COMP_0_GND) {
      _fwdRaw = _adc.getResult_mV();
      _fwd = convertFwd(_fwdRaw);
      _adcChannel = ADS1115_COMP_1_GND;
    } else {
      _rflRaw = _adc.getResult_mV();
      _rfl = convertRfl(_rflRaw);
      _adcChannel = ADS1115_COMP_0_GND;
    }
  }

  virtual void init() override {
    Wire.begin(SDA, SCL, 800000);
    pinMode(_alertReadyPin, INPUT_PULLUP);
    _adc.init();
    _adc.setVoltageRange_mV(ADS1115_RANGE_4096);
    _adc.setConvRate(ADS1115_16_SPS);
    _adc.setMeasureMode(ADS1115_SINGLE);
    _adc.setAlertPinMode(ADS1115_ASSERT_AFTER_1);
    _adc.setAlertPinToConversionReady();
    start();
    _LOGI(_name, "ADS1115 initialized and conversion is started.");
  };

  virtual void loop() override {
    if (digitalRead(_alertReadyPin) != HIGH) {
      read();
      start();
      _conversionnCount++;
      if (_cnt) {
        _cnt--;
      }
    }
  }

  virtual void getStatus(JsonObject &doc) const override {
    auto node = doc["sensor"][_name];
    node["cnt"] = _conversionnCount;
    node["fwdRaw"] = _fwdRaw;
    node["rflRaw"] = _rflRaw;
    node["fwdIntercept"] = _fwdIntercept;
    node["rflIntercept"] = _rflIntercept;
    SWRMeter::getStatus(doc);
  };

  virtual void setConfig(const JsonObject &doc) override {
    SWRMeter::setConfig(doc);
    auto node = doc["sensor"][_name];
    if (!node["fwdIntercept"].isNull()) {
      _fwdIntercept = node["fwdIntercept"];
    }
    if (!node["rflIntercept"].isNull()) {
      _rflIntercept = node["rflIntercept"];
    }
  };

  virtual float getTarget() const override { return _rflRaw / _fwdRaw; };
};

} // namespace Sensor