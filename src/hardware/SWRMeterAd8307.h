#pragma once

#include "debug.h"
#include "sensor/SWRMeter.h"
#include "sensor/Sensor.h"
#include <ADS1115_WE.h>
#include <Arduino.h>
#include <Wire.h>

namespace Sensor {

class SWRMeterAd8307 : public SWRMeter {

private:
  ADS1115_WE *_adc;
  uint8_t _alertReadyPin;
  ADS1115_MUX _adcChannel = ADS1115_COMP_0_GND;

public:
  SWRMeterAd8307(etl::message_router_id_t id, int addr, uint8_t alertReadyPin) : SWRMeter(id, "SWRMeterAd8307") {
    static ADS1115_WE __adc(addr);
    _adc = &__adc;
    _alertReadyPin = alertReadyPin;
  };

  void start() {
    _adc->setCompareChannels(_adcChannel);
    _adc->clearAlert();
    _adc->startSingleMeasurement();
  }

  float convertFwd(float input) { return input; }

  float convertRfl(float input) { return input; }

  void read() {
    if (_adcChannel == ADS1115_COMP_0_GND) {
      _fwd = convertFwd(_adc->getResult_mV());
      _adcChannel = ADS1115_COMP_1_GND;
    } else {
      _rfl = convertRfl(_adc->getResult_mV());
      _adcChannel = ADS1115_COMP_0_GND;
    }
  }

  virtual void init() {
    Wire.begin(SDA, SCL, 800000);
    pinMode(_alertReadyPin, INPUT_PULLUP);
    _adc->init();
    _adc->setVoltageRange_mV(ADS1115_RANGE_4096);
    _adc->setConvRate(ADS1115_16_SPS);
    _adc->setMeasureMode(ADS1115_SINGLE);
    _adc->setAlertPinMode(ADS1115_ASSERT_AFTER_1);
    _adc->setAlertPinToConversionReady();
    start();
    _LOGI(_name, "ADS1115 initialized and conversion is started.");
  };

  virtual void loop() {
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