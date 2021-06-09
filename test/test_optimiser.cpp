#undef SYSLOG_SERVER

#include "actuators/ActuatorDACGPIO.h"
#include "config.h"
#include "device/ATU.h"
#include <Arduino.h>
#include <unity.h>

#define K17 18 // 5 pF        1    // ESP32 GPI018
#define K18 5  // 10 pF       2    // ESP32 GPI05
#define K19 17 // 22 pF       4    // ESP32 GPI017
#define K20 16 // 39 pF       8    // ESP32 GPI016
#define K21 4  // 78 pF       16   // ESP32 GPI04
#define K22 2  // 160 pF      32   // ESP32 GPI02
#define K23 15 // 330 pF      64   // ESP32 GPI015
#define K24 23 // 660 pF      128  // ESP32 GPI023

namespace Test {

using namespace Sensor;
using namespace Core;
using namespace Actuators;

ActuatorDACGPIO actuatorC2("test", {pin : {K17, K18, K19, K20, K21, K22, K23, K24}}, 1, 255);

class TestSWRMeter : public Sensor::SWRMeter {
private:
  Actuator &_actuator;
  uint32_t _measurementCount;

public:
  TestSWRMeter(Actuator &actuator) : SWRMeter(Core::COMPONENT_CLASS_SENSOR, "test"), _actuator(actuator){};
  virtual bool isReady() const override { return true; };
  virtual float getTarget() const override {
    return pow((_actuator.getValue() - 100.0), 2) / 1000.0;
  };
  void clearMeasurementCount() { _measurementCount = 0; }
  virtual void startMeasurementCycle(uint8_t cnt) override { _measurementCount++; };
  uint32_t getMeasurementCount() { return _measurementCount; }
};

TestSWRMeter swr(actuatorC2);

class TestAtu : public Device::ATU {
private:
  Actuator &_actuator;

public:
  TestAtu(Actuator &actuator) : Device::ATU(COMPONENT_CLASS_GENERIC, swr), _actuator(actuator) {
  }
  virtual void testOptimise(int16_t step) {
    optimise(_actuator, step, 0.0);
  };
};

TestAtu atu(actuatorC2);

} // namespace Test

Core::ComponentManager mgr;

void testOptimiseWithTinySteps() {
  Test::swr.clearMeasurementCount();
  Test::actuatorC2.setValue(128);
  Test::atu.testOptimise(1);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  Test::atu.testOptimise(1);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  Test::atu.testOptimise(1);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  _LOGI("test", "Measurements: %d", Test::swr.getMeasurementCount());
}

void testOptimiseWithSmallSteps() {
  Test::swr.clearMeasurementCount();
  Test::actuatorC2.setValue(128);
  Test::atu.testOptimise(2);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  Test::atu.testOptimise(2);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  Test::atu.testOptimise(2);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  _LOGI("test", "Measurements: %d", Test::swr.getMeasurementCount());
}

void testOptimiseWithBiggerSteps() {
  Test::swr.clearMeasurementCount();
  Test::actuatorC2.setValue(128);
  Test::atu.testOptimise(4);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  Test::atu.testOptimise(4);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  Test::atu.testOptimise(4);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  _LOGI("test", "Measurements: %d", Test::swr.getMeasurementCount());
}

void testOptimiseLowLimit() {
  Test::swr.clearMeasurementCount();
  Test::actuatorC2.setValue(0);
  Test::atu.testOptimise(-4);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  _LOGI("test", "Measurements: %d", Test::swr.getMeasurementCount());
}

void testOptimiseHighLimit() {
  Test::swr.clearMeasurementCount();
  Test::actuatorC2.setValue(255);
  Test::atu.testOptimise(4);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  Test::actuatorC2.setValue(253);
  Test::atu.testOptimise(4);
  TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  _LOGI("test", "Measurements: %d", Test::swr.getMeasurementCount());
}

void testOptimiseStability() {
  Test::swr.clearMeasurementCount();
  for (int i = 0; i <= 255; i++) {
    Test::actuatorC2.setValue(i);
    Test::atu.testOptimise(5);
    TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  }
  _LOGI("test", "Measurements: %d", Test::swr.getMeasurementCount());
}

void testOptimiseStabilityStepSize() {
  Test::swr.clearMeasurementCount();
  for (int i = 1; i <= 255; i++) {
    Test::actuatorC2.setValue(0);
    Test::atu.testOptimise(i);
    TEST_ASSERT_EQUAL(100, Test::actuatorC2.getValue());
  }
  _LOGI("test", "Measurements: %d", Test::swr.getMeasurementCount());
}

void setup() {
  UNITY_BEGIN();
  Serial.begin(921600);
  _LOGI("setup", "Start");
  debugInit();
  Test::actuatorC2.init();
  Test::swr.init();
  Test::atu.init();
  mgr.init();
  RUN_TEST(testOptimiseWithTinySteps);
  RUN_TEST(testOptimiseWithSmallSteps);
  RUN_TEST(testOptimiseWithBiggerSteps);
  RUN_TEST(testOptimiseLowLimit);
  RUN_TEST(testOptimiseHighLimit);
  RUN_TEST(testOptimiseStability);
  RUN_TEST(testOptimiseStabilityStepSize);
  UNITY_END();
}

void loop() {
  debugLoop();
  mgr.loop();
}