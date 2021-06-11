#undef SYSLOG_SERVER

#include "debug.h"
#include "model/TMatchModel.h"
#include <Arduino.h>
#include <unity.h>

using namespace Model;

TMatchModel model;

void initTests() {
  _LOGI("setup", "Start");
  debugInit();
}

void testModel() {
  model.setFreq(1.83);
  model.setLoad(10.0, 0.0);
  model.setComponents(140.45026012886674, 19.425302153551996, 249.99999999999994);
  _LOGI("model", "SWR=%f", model.getSwr());
  _LOGI("model", "Loss=%f", model.getLoss());
  TEST_ASSERT_DOUBLE_WITHIN(0.0001, 1.0, model.getSwr());
  TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.362792, model.getLoss());
}

void runTests() {
  RUN_TEST(testModel);
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
}

#if defined(NATIVE)
using namespace fakeit;

int main() {
  UNITY_BEGIN();
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  When(Method(ArduinoFake(), delay)).AlwaysReturn();
  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  initTests();
  runTests();
  return UNITY_END();
}
#endif