#pragma once

#include "debug.h"
#include "sensor/SWRMeter.h"
#include "sensor/Sensor.h"
#include <ADS1115_WE.h>
#include <Arduino.h>
#include <Wire.h>

namespace Sensor {

class SWRMeterAds1115Ad8310 : public SWRMeter {

private:
  ADS1115_WE &_adc;
  const uint8_t _alertReadyPin;
  ADS1115_MUX _adcChannel = ADS1115_COMP_0_GND;
  float _fwdRaw;
  float _rflRaw;
  uint32_t _conversionnCount = 0;

  ADS1115_WE &getAdc(int addr) {
    static ADS1115_WE __adc(addr);
    return __adc;
  }

public:
  SWRMeterAds1115Ad8310(etl::message_router_id_t id,
                        int addr,
                        uint8_t alertReadyPin) : SWRMeter(id, "SWRMeterAds1115Ad8310"),
                                                 _adc(getAdc(addr)),
                                                 _alertReadyPin(alertReadyPin){};

  void start() {
    _adc.setCompareChannels(_adcChannel);
    _adc.clearAlert();
    _adc.startSingleMeasurement();
  }

  float convertFwd(const float x) const { return exp((x - 1913.417895) / 102.743582); }

  float convertRfl(const float x) const { return exp((x - 1913.151197) / 101.409361); }

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

  virtual void getStatus(JsonDocument &doc) const override {
    doc["sensor"][_name]["cnt"] = _conversionnCount;
    doc["sensor"][_name]["fwdRaw"] = _fwdRaw;
    doc["sensor"][_name]["rflRaw"] = _rflRaw;
    SWRMeter::getStatus(doc);
  };

  virtual float getTarget() const override { return _rflRaw / _fwdRaw; };

};

} // namespace Sensor