#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "core/Component.h"
#include "network/webpage.h"

namespace Network {

#define LOG_BUFF_SIZE (WS_MAX_QUEUED_MESSAGES - 1)

void __debug_transport_web_callback(const char *s);

class WEB : public Core::Component {
private:
  AsyncWebServer *_server = nullptr;
  AsyncWebSocket *_ws = nullptr;
  debug_transport_t __debug_transport_chain;
  String _logBuff[LOG_BUFF_SIZE];
  uint8_t _logBuffHead = 0;
  uint8_t _logBuffTail = 0;
  char *_cmd = nullptr;
  AsyncWebSocketClient *_client = nullptr;

  const String &buffPush(const String &val) {
    _logBuff[_logBuffHead] = String(val);
    _logBuffHead = ((uint8_t)(_logBuffHead + 1) % LOG_BUFF_SIZE);
    if (_logBuffHead == _logBuffTail) {
      _logBuffTail = ((uint8_t)(_logBuffTail + 1) % LOG_BUFF_SIZE);
    }
    return val;
  }

protected:
  virtual void sendStatus(AsyncWebSocketClient *client = NULL) {
    StaticJsonDocument<1024> doc;
    JsonObject obj = doc.to<JsonObject>();
    doc["topic"] = "status";
    getGlobalStatus(obj);
    String output;
    serializeJson(doc, output);
    if (client == NULL) {
      _ws->textAll(output);
    } else {
      client->text(output);
    }
  }

  virtual void logMessade(const String &message) {
    StaticJsonDocument<512> doc;
    doc["topic"] = "log";
    doc["message"] = message;
    String output;
    serializeJson(doc, output);
    _ws->textAll(output);
  }

  virtual void sendConfig(AsyncWebSocketClient *client) {
    StaticJsonDocument<1024> doc;
    JsonObject obj = doc.to<JsonObject>();
    getGlobalInfo(obj);
    obj["topic"] = "config";
    String output;
    serializeJson(doc, output);
    client->text(output);
  }

  virtual void sendLogBuffer(AsyncWebSocketClient *client) {
    uint8_t ptr = _logBuffTail;
    while (ptr != _logBuffHead) {
      StaticJsonDocument<512> doc;
      doc["topic"] = "log";
      doc["message"] = _logBuff[ptr];
      String output;
      serializeJson(doc, output);
      client->text(output);
      ptr = (uint8_t)(ptr + 1) % LOG_BUFF_SIZE;
    }
  }

  virtual void handleRequest(AsyncWebSocketClient *client, const JsonObject &doc) {
    String response;
    if (!doc["command"].isNull()) {
      String cmd = doc["command"];
      if (cmd == "ping") {
        response = "{\"command\":\"pong\"}";
        client->text(response);
        return;
      }
      auto c = parseCommand(cmd);
      if (c != Core::COMMAND_TYPE_UNKNOWN) {
        broadcastCommand(c, doc);
        sendStatus(client);
      } else {
        response = "{\"error\":\"Unknown command\"}";
        client->text(response);
      }
    }
  }

public:
  WEB() : Core::Component(Core::COMPONENT_CLASS_NETWORK) {
    static AsyncWebServer server(80);
    _server = &server;
    _ws = new AsyncWebSocket("/dashws");
    _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      // respond with the compressed frontend
      AsyncWebServerResponse *response =
          request->beginResponse_P(200, "text/html", DASH_HTML, DASH_HTML_SIZE);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    });
    _ws->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client,
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
      if (type == WS_EVT_CONNECT) {
        sendConfig(client);
        sendStatus(client);
        sendLogBuffer(client);
      }
      if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len) {
          if (info->opcode == WS_TEXT) {
            if (_cmd != nullptr) {
              free(_cmd);
              _cmd = nullptr;
            }
            _cmd = (char *)malloc(len + 1);
            bzero(_cmd, len + 1);
            bcopy(data, _cmd, len);
            _client = client;
          }
        }
      }
    });
    _server->addHandler(_ws);
    __debug_transport_chain =
        set_debug_transport(&__debug_transport_web_callback);
  };

  virtual void init() override { _server->begin(); };

  virtual void timer250() override { sendStatus(); }

  virtual void loop() override {
    if (_cmd != nullptr) {
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, _cmd);
      free(_cmd);
      _cmd = nullptr;
      if (!error) {
        doc.shrinkToFit();
        JsonObject obj = doc.as<JsonObject>();
        handleRequest(_client, obj);
        doc.clear();
      } else {
        _LOGE("ws", "WS request deserialisation failure: %s",
              error.c_str());
      }
    }
  }

  virtual void log(const char *s) {
    logMessade(buffPush(String(s)));
    __debug_transport_chain(s);
  }
};

WEB web;

void __debug_transport_web_callback(const char *s) { web.log(s); }

} // namespace Network