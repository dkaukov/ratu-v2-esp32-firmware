#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "core/Component.h"
#include "network/webpage.h"

namespace Network {

#define LOG_BUFF_SIZE (WS_MAX_QUEUED_MESSAGES - 1)

void __debug_transport_web_callback(const char* s);

class WEB : public Core::Component {
 private:
  AsyncWebServer* _server = nullptr;
  AsyncWebSocket* _ws = nullptr;
  debug_transport_t __debug_transport_chain;
  String _logBuff[LOG_BUFF_SIZE];
  uint8_t _logBuffHead = 0;
  uint8_t _logBuffTail = 0;

  const String& buffPush(const String& val) {
    _logBuff[_logBuffHead] = String(val);
    _logBuffHead = ((uint8_t)(_logBuffHead + 1) % LOG_BUFF_SIZE);
    if (_logBuffHead == _logBuffTail) {
      _logBuffTail = ((uint8_t)(_logBuffTail + 1) % LOG_BUFF_SIZE);
    }
    return val;
  }

 protected:
  virtual void sendStatus() {
    StaticJsonDocument<1024> doc;
    JsonObject obj = doc.to<JsonObject>();
    doc["topic"] = "status";
    getGlobalStatus(obj);
    String output;
    serializeJson(doc, output);
    _ws->textAll(output);
  }

  virtual void logMessade(const String& message) {
    StaticJsonDocument<512> doc;
    doc["topic"] = "log";
    doc["message"] = message;
    String output;
    serializeJson(doc, output);
    _ws->textAll(output);
  }

  virtual void sendConfig(uint32_t id) {
    StaticJsonDocument<1024> doc;
    doc["topic"] = "config";
    doc["device"]["id"] = getDeviceId();
    doc["device"]["heapSize"] = ESP.getHeapSize();
    doc["device"]["sdkVersion"] = ESP.getSdkVersion();
    doc["device"]["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    doc["device"]["sketchMD5"] = ESP.getSketchMD5();
    doc["device"]["chipModel"] = ESP.getChipModel();

    doc["device"]["wifi"]["ssid"] = WiFi.SSID();
    doc["device"]["wifi"]["bssid"] = WiFi.BSSIDstr();
    doc["device"]["wifi"]["ip"] = net.localIP().toString();
    doc["device"]["wifi"]["hostname"] = WiFi.getHostname();
    doc["device"]["wifi"]["mqtt-local-port"] = net.localPort();

    String output;
    serializeJson(doc, output);
    _ws->text(id, output);
  }

  virtual void sendLogBuffer(uint32_t id) {
    uint8_t ptr = _logBuffTail;
    while (ptr != _logBuffHead) {
      StaticJsonDocument<512> doc;
      doc["topic"] = "log";
      doc["message"] = _logBuff[ptr];
      String output;
      serializeJson(doc, output);
      _ws->text(id, output);
      ptr = (uint8_t)(ptr + 1) % LOG_BUFF_SIZE;
    }
  }

  virtual void handleRequest(uint32_t from, const JsonObject& doc) {
    String response;
    if (doc["command"] == "ping") {
      response = "{\"command\":\"pong\"}";
    }
    _ws->text(from, response);
  }

 public:
  WEB() : Core::Component(Core::COMPONENT_CLASS_NETWORK) {
    static AsyncWebServer server(80);
    _server = &server;
    _ws = new AsyncWebSocket("/dashws");
    _server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
      // respond with the compressed frontend
      AsyncWebServerResponse* response =
          request->beginResponse_P(200, "text/html", DASH_HTML, DASH_HTML_SIZE);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    });
    _ws->onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client,
                     AwsEventType type, void* arg, uint8_t* data, size_t len) {
      if (type == WS_EVT_CONNECT) {
        sendConfig(client->id());
        sendLogBuffer(client->id());
      }
      if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
          if (info->opcode == WS_TEXT) {
            DynamicJsonDocument doc(ESP.getMaxAllocHeap() - 1024);
            DeserializationError error = deserializeJson(doc, data, len);
            if (!error) {
              doc.shrinkToFit();
              JsonObject obj = doc.as<JsonObject>();
              handleRequest(client->id(), obj);
              doc.clear();
            } else {
              _LOGE("ws", "WS request deserialisation failure: %s",
                    error.c_str());
            }
          }
        }
      }
    });
    _server->addHandler(_ws);
    __debug_transport_chain =
        set_debug_transport(&__debug_transport_web_callback);
  };

  virtual void init() override { _server->begin(); };

  virtual void timer1000() override { sendStatus(); }

  virtual void log(const char* s) {
    logMessade(buffPush(String(s)));
    __debug_transport_chain(s);
  }
};

WEB web;

void __debug_transport_web_callback(const char* s) { web.log(s); }

}  // namespace Network