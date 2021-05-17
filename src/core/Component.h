#pragma once

#define ETL_MESSAGE_TIMER_USE_INTERRUPT_LOCK
#define ETL_MESSAGE_TIMER_ENABLE_INTERRUPTS \
  {}
#define ETL_MESSAGE_TIMER_DISABLE_INTERRUPTS \
  {}

#include "debug.h"
#include "etl/list.h"
#include "etl/message.h"
#include "etl/message_bus.h"
#include "etl/message_router.h"
#include "etl/message_timer.h"
#include "etl/timer.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

namespace Core {

enum {
  MESSAGE_LOOP,
  MESSAGE_GET_STATUS,
  MESSAGE_SET_CONFIG,
  MESSAGE_TIMER,
};

enum {
  COMPONENT_CLASS_GENERIC,
  COMPONENT_CLASS_SENSOR,
  COMPONENT_CLASS_ACTUATOR,
  COMPONENT_CLASS_COMMUNICATION,
  COMPONENT_CLASS_POWER_MANAGER,
};

typedef enum {
  TIMER_TYPE_250_MS,
  TIMER_TYPE_1000_MS,
  TIMER_TYPE_COUNT
} timer_type_t;

struct MessageLoop : public etl::message<MESSAGE_LOOP> {};
static MessageLoop msg_loop = {};

struct MessageGetStatus : public etl::message<MESSAGE_GET_STATUS> {
  MessageGetStatus(JsonDocument &doc_) : doc(doc_){};
  JsonDocument &doc;
};

struct MessageSetConfig : public etl::message<MESSAGE_SET_CONFIG> {
  MessageSetConfig(const JsonDocument &doc_) : doc(doc_){};
  const JsonDocument &doc;
};

struct MessageTimer : public etl::message<MESSAGE_TIMER> {
  MessageTimer(const timer_type_t type) : type(type){};
  const timer_type_t type;
};

typedef etl::message_bus<16> MessageBus_t;
typedef etl::message_timer<TIMER_TYPE_COUNT> MessageTimer_t;

static MessageBus_t bus;
static MessageTimer_t timer;

class Component : public etl::message_router<Component, MessageLoop, MessageGetStatus, MessageSetConfig, MessageTimer> {
private:
  MessageBus_t *_bus;

public:
  Component(etl::message_router_id_t id) : message_router(id) {
    _bus = &bus;
    _bus->subscribe(*this);
  };

  void on_receive_unknown(const etl::imessage &msg){};
  void on_receive_unknown(etl::imessage_router &sender, const etl::imessage &msg){};

  void on_receive(etl::imessage_router &sender, const MessageLoop &msg) { loop(); }
  void on_receive(const MessageLoop &msg) { loop(); };

  void on_receive(etl::imessage_router &sender, const MessageGetStatus &msg) { getStatus(msg.doc); }
  void on_receive(const MessageGetStatus &msg) { getStatus(msg.doc); };

  void on_receive(etl::imessage_router &sender, const MessageSetConfig &msg) { setConfig(msg.doc); }
  void on_receive(const MessageSetConfig &msg) { setConfig(msg.doc); };

  void on_receive(etl::imessage_router &sender, const MessageTimer &msg) { on_receive(msg); }
  void on_receive(const MessageTimer &msg) {
    switch (msg.type) {
    case TIMER_TYPE_250_MS:
      timer250();
      break;
    case TIMER_TYPE_1000_MS:
      timer1000();
      break;
    default:
      break;
    }
  };

  virtual void getStatus(JsonDocument &doc) const {};
  virtual void setConfig(const JsonDocument &doc){};

  virtual void init(){};
  virtual void loop(){};
  virtual void timer250(){};
  virtual void timer1000(){};
  void yeld() const { _bus->receive(msg_loop); };
  void busyWait(std::function<bool(void)> busy) {
    while (busy()) {
      yeld();
    }
  }
};

class ComponentManager {
private:
  MessageBus_t *_bus;
  MessageTimer_t *_timer;
  etl::timer::id::type _tm250id;
  etl::timer::id::type _tm1000id;

public:
  ComponentManager() {
    _bus = &bus;
    _timer = &timer;
  }

  void init() {
    static MessageTimer tm250 = {TIMER_TYPE_250_MS};
    static MessageTimer tm1000 = {TIMER_TYPE_1000_MS};
    _tm250id = _timer->register_timer(tm250, bus, 250, true);
    _tm1000id = _timer->register_timer(tm1000, bus, 1000, true);
    _timer->enable(true);
    _timer->start(_tm250id);
    _timer->start(_tm1000id);
  };

  void loop() const {
    static uint32_t lastMillis = millis();
    uint32_t currentMillis = millis();
    _timer->tick(currentMillis - lastMillis);
    lastMillis = currentMillis;
    _bus->receive(msg_loop);
  };

  void getStatus(JsonDocument &doc) const {
    static MessageGetStatus msg = {doc};
    _bus->receive(msg);
  };
};

} // namespace Core