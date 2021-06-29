#pragma once

#include <ESPAsyncWebServer.h>

#include "core/Component.h"
#include "network/webpage.h"

namespace Network {

class WEB : public Core::Component {
 private:
  AsyncWebServer* _server = nullptr;
  AsyncWebSocket* _ws = nullptr;

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
    _server->addHandler(_ws);
  };

  virtual void init() override { _server->begin(); };
  virtual void loop() override{};
};

WEB web;

}  // namespace Network