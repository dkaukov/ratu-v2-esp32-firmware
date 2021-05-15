#pragma once

#include "debug.h"
#include "etl/list.h"
#include "etl/message.h"
#include "etl/message_bus.h"
#include "etl/message_router.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

namespace Core {

enum {
  MESSAGE_LOOP,
  MESSAGE_GET_STATUS,
  MESSAGE_SET_CONFIG,
};

enum {
  COMPONENT_CLASS_GENERIC,
  COMPONENT_CLASS_SENSOR,
  COMPONENT_CLASS_ACTUATOR,
  COMPONENT_CLASS_COMMUNICATION,
};

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

static etl::message_bus<100> bus;

class Component : public etl::message_router<Component, MessageLoop, MessageGetStatus, MessageSetConfig> {
private:
  etl::message_bus<100> *_bus;

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

  virtual void getStatus(JsonDocument &doc){};
  virtual void setConfig(const JsonDocument &doc){};

  virtual void init(){};
  virtual void loop(){};
  void yeld() { _bus->receive(msg_loop); };
  void busyWait(std::function<bool(void)> busy) {
    while (busy()) {
      yeld();
    }
  }
};

class ComponentManager {
private:
  etl::message_bus<100> *_bus;

public:
  ComponentManager() {
    _bus = &bus;
  }

  void init(){};

  void loop() {
    _bus->receive(msg_loop);
  };

  void getStatus(JsonDocument &doc) {
    static MessageGetStatus msg = {doc};
    _bus->receive(msg);
  };
};

} // namespace Core