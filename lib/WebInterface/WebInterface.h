#pragma once

#include <ESPAsyncWebServer.h>

#include "FanControlService.h"

// HTTP-Schnittstelle: liefert das Webfrontend (LittleFS) aus und stellt eine
// REST-API zum Lesen/Schreiben der Konfiguration sowie zum Abfragen des
// Status bereit. Einzige Verantwortung: HTTP <-> FanControlService.
class WebInterface {
 public:
  explicit WebInterface(FanControlService& service, uint16_t port = 80);

  void begin();

 private:
  void registerRoutes();
  void handleGetConfig(AsyncWebServerRequest* request) const;
  void handleGetStatus(AsyncWebServerRequest* request) const;
  void handlePostConfig(AsyncWebServerRequest* request, const JsonVariant& body);

  FanControlService& service;
  AsyncWebServer server;
};
