#pragma once

#include <ESPAsyncWebServer.h>

#include "core/Component.h"
#include "network/webpage.h"

namespace Network {

class WEB : public Core::Component {
 private:
  AsyncWebServer* _server = nullptr;
  AsyncWebSocket* _ws = nullptr;

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
      }
    });
    _server->addHandler(_ws);
  };

  virtual void init() override { _server->begin(); };
  virtual void loop() override{};
  virtual void timer1000() override { sendStatus(); }
};

WEB web;

}  // namespace Network