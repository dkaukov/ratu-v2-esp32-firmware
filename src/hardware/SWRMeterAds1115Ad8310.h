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

  float convertFwd(float input) const { return input; }

  float convertRfl(float input) const { return input; }

  void read() {
    if (_adcChannel == ADS1115_COMP_0_GND) {
      _fwd = convertFwd(_adc.getResult_mV());
      _adcChannel = ADS1115_COMP_1_GND;
    } else {
      _rfl = convertRfl(_adc.getResult_mV());
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
      if (_cnt) {
        _cnt--;
      }
    }
  }
};

} // namespace Sensor