#undef SYSLOG_SERVER

#include "device/TMatch.h"
#include "model/TMatchModel.h"
#include <Arduino.h>
#include <unity.h>

#define L_RANGE_IN_STEPS 2964 * 15
#define C1_RANGE_IN_STEPS 2964 / 2

#define L_ACTUATOR_MIN 0.03
#define L_ACTUATOR_MAX 11.9

#define C1_ACTUATOR_MIN 150.0 + 360.0
#define C1_ACTUATOR_MAX 150.0 + 5.0

namespace Test {

using namespace Sensor;
using namespace Core;
using namespace Actuators;
using namespace Model;
using namespace Device;

class TestActuator : public Actuator {
private:
  int32_t _value = 0;

public:
  TestActuator(const uint32_t maxSteps,
               const char *name,
               const float min,
               const float max) : Actuator(Core::COMPONENT_CLASS_ACTUATOR, name) {
    _max = maxSteps;
    _a = (max - min) / (_max - _min);
    _b = min - _min * _a;

    __a = 1.0 / _a;
    __b = _b / _a;
    _inverted = min > max;
  };
  virtual int32_t getValue() const override { return _value; };

  virtual int32_t setValue(int32_t pos) override {
    int32_t oldPosition = _value;
    pos = constrain(pos, _min, _max);
    _value = pos;
    return pos - oldPosition;
  };

  virtual bool isReady() const override { return true; }
};

TestActuator actuatorL(L_RANGE_IN_STEPS, "L", L_ACTUATOR_MIN, L_ACTUATOR_MAX);
TestActuator actuatorC1(C1_RANGE_IN_STEPS, "C1", C1_ACTUATOR_MIN, C1_ACTUATOR_MAX);
TestActuator actuatorC2(C1_RANGE_IN_STEPS, "C2", C1_ACTUATOR_MIN, C1_ACTUATOR_MAX);
TMatchModel model;

class TestSWRMeter : public SWRMeter {
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
    return getRho() /*+ (_actuatorL.getValue() / 256.0)*/;
  };
  virtual float getRho() const override {
    _actuatorC2.setValue(_actuatorC1.getValue());
    model.setComponents(_actuatorC1.getPhisicalValue(), _actuatorL.getPhisicalValue(), _actuatorC1.getPhisicalValue());
    return model.getRho();
  };
  void clearMeasurementCount() { _measurementCount = 0; }
  virtual void startMeasurementCycle(uint8_t cnt) override { _measurementCount++; };
  uint32_t getMeasurementCount() { return _measurementCount; }
};

TestSWRMeter swr(actuatorC1, actuatorC2, actuatorL);

class TestAtu : public Device::LMatch {

public:
  TestAtu() : Device::LMatch(swr, actuatorL, actuatorC1) {}
  virtual void testOptimise(int16_t step) {
    _actuatorC1InitialStep = step;
    _actuatorLInitialStep = step;
    tune();
  };
  void setTuningMode(atu_tuning_type_t mode) {
    _tuningMode = mode;
  }
};

TestAtu atu;

} // namespace Test

Core::ComponentManager mgr;

void printState() {
  printf("\n");
  printf("state: %s=%f(%d), %s=%f(%d)\n",
         Test::actuatorC1.getName(), Test::actuatorC1.getPhisicalValue(), Test::actuatorC1.getValue(),
         Test::actuatorL.getName(), Test::actuatorL.getPhisicalValue(), Test::actuatorL.getValue());
  printf("SWR         = %f\n", Test::model.getSwr());
  printf("Loss        = %f%%\n", Test::model.getLoss() * 100.0);
  printf("Est.Loss    = %f%%\n", Model::TMatchModel::estimateLoss(Test::model.getFreq(), Test::actuatorC1.getPhisicalValue(), Test::actuatorL.getPhisicalValue(), 100.0) * 100.0);
  printf("Measurements: %d\n", Test::swr.getMeasurementCount());
  printf("\n");
}

void testDummyLoad_50_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(50, 0);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorL.setPct(0.0);
  Test::atu.testOptimise(50);
  printState();
}

void testDummyLoad_33_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(33, 0);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorL.setPct(0.0);
  Test::atu.testOptimise(50);
  printState();
}

void testDummyLoad_100_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(100, 0);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorL.setPct(0.0);
  Test::atu.testOptimise(50);
  printState();
}

void testSomething_5010_7020() {
  Test::model.setFreq(7.020);
  Test::model.setLoad(50, -50);
  Test::swr.clearMeasurementCount();
  Test::actuatorC1.setPct(1.0);
  Test::actuatorL.setPct(1.0);
  Test::atu.testOptimise(60);
  printState();
}

void initTests() {
  _LOGI("setup", "Start");
  debugInit();
  Test::actuatorC1.init();
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