#undef SYSLOG_SERVER

#include "actuators/ActuatorDACGPIO.h"
#include "device/TMatch.h"
#include "model/TMatchModel.h"
#include <Arduino.h>
#include <unity.h>

#define L_ACTUATOR_MIN 0.05
#define L_ACTUATOR_MAX 12.75

#define C1_ACTUATOR_MIN 5.0
#define C1_ACTUATOR_MAX 1304.0

#define C2_ACTUATOR_MIN 5.0
#define C2_ACTUATOR_MAX 1304.0

namespace Test {

using namespace Sensor;
using namespace Core;
using namespace Actuators;
using namespace Model;

ActuatorDACGPIO actuatorC1("C1", {pin : {0, 0, 0, 0, 0, 0, 0, 0}}, C1_ACTUATOR_MIN, C1_ACTUATOR_MAX);
ActuatorDACGPIO actuatorC2("C2", {pin : {0, 0, 0, 0, 0, 0, 0, 0}}, C2_ACTUATOR_MIN, C2_ACTUATOR_MAX);
ActuatorDACGPIO actuatorL("L", {pin : {0, 0, 0, 0, 0, 0, 0, 0}}, L_ACTUATOR_MIN, L_ACTUATOR_MAX);
TMatchModel model;

class TestSWRMeter : public Sensor::SWRMeter {
private:
  Actuator &_actuatorC1;
  Actuator &_actuatorC2;
  Actuator &_actuatorL;
  uint32_t _measurementCount;

public:
  TestSWRMeter(Actuator &actuatorC1, Actuator &actuatorC2, Actuator &actuatorL)
      : SWRMeter(Core::COMPONENT_CLASS_SENSOR, "test"),
        _actuatorC1(actuatorC1),
        _actuatorC2(actuatorC2),
        _actuatorL(actuatorL){};
  virtual bool isReady() const override { return true; };
  virtual float getTarget() const override {
    model.setComponents(_actuatorC1.getPhisicalValue(), _actuatorL.getPhisicalValue(), _actuatorC2.getPhisicalValue());
    return model.getRho() /*+ (_actuatorL.getValue() / 256.0)*/;
  };
  void clearMeasurementCount() { _measurementCount = 0; }
  virtual void startMeasurementCycle(uint8_t cnt) override { _measurementCount++; };
  uint32_t getMeasurementCount() { return _measurementCount; }
};

TestSWRMeter swr(actuatorC1, actuatorC2, actuatorL);

class TestAtu : public Device::TMatch {

public:
  TestAtu() : Device::TMatch(swr, actuatorL, actuatorC1, actuatorC2) {}
  virtual void testOptimise(int16_t step) {
    _actuatorC1InitialStep = step;
    _actuatorC2InitialStep = step;
    _actuatorLInitialStep = step;
    tune();
  };
};

TestAtu atu;

} // namespace Test

Core::ComponentManager mgr;

void printState() {
  printf("\n");
  printf("state: %s=%f(%d), %s=%f(%d), %s=%f(%d)\n",
         Test::actuatorC1.getName(), Test::actuatorC1.getPhisicalValue(), Test::actuatorC1.getValue(),
         Test::actuatorL.getName(), Test::actuatorL.getPhisicalValue(), Test::actuatorL.getValue(),
         Test::actuatorC2.getName(), Test::actuatorC2.getPhisicalValue(), Test::actuatorC2.getValue());
  printf("SWR=%f\n", Test::model.getSwr());
  printf("Loss=%f\n", Test::model.getLoss() * 100.0);
  printf("Measurements: %d\n", Test::swr.getMeasurementCount());
  printf("\n");
}

void testDummyLoad_50_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(50, 0);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorC2.setPct(1.0);
  Test::actuatorL.setPct(0.0);
  Test::atu.testOptimise(4);
  printState();
}

void testDummyLoad_33_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(33, 0);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorC2.setPct(1.0);
  Test::actuatorL.setPct(0);
  Test::atu.testOptimise(4);
  printState();
}

void testDummyLoad_100_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(100, 0);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorC2.setPct(1.0);
  Test::actuatorL.setPct(0);
  Test::atu.testOptimise(4);
  printState();
}

void testSomething_5010_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(50, -50);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorC2.setPct(1.0);
  Test::actuatorL.setPct(0);
  Test::atu.testOptimise(4);
  printState();
}

void initTests() {
  _LOGI("setup", "Start");
  debugInit();
  Test::actuatorC1.init();
  Test::actuatorC2.init();
  Test::actuatorL.init();
  Test::swr.init();
  Test::atu.init();
  mgr.init();
}

void runTests() {
  RUN_TEST(testDummyLoad_50_7020);
  RUN_TEST(testDummyLoad_33_7020);
  RUN_TEST(testDummyLoad_100_7020);
  RUN_TEST(testSomething_5010_7020);
}

void setup() {
  UNITY_BEGIN();
  Serial.begin(921600);
  initTests();
  runTests();
  UNITY_END();
}

void loop() {
  debugLoop();
  mgr.loop();
}

#if defined(NATIVE)
using namespace fakeit;

int main() {
  UNITY_BEGIN();
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  When(Method(ArduinoFake(), delay)).AlwaysReturn();
  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  When(Method(ArduinoFake(), millis)).AlwaysReturn();
  initTests();
  runTests();
  return UNITY_END();
}
#endif