#pragma once

#include <ESPAsyncWebServer.h>

#include "FanControlService.h"
#include "OtaUpdater.h"

// HTTP-Schnittstelle: liefert das Webfrontend (LittleFS) aus und stellt eine
// REST-API zum Lesen/Schreiben der Konfiguration, zum Abfragen des Status sowie
// einen passwortgeschützten OTA-Upload bereit.
class WebInterface {
 public:
  WebInterface(FanControlService& service, OtaUpdater& otaUpdater,
               const char* otaUsername, const char* otaPassword,
               uint16_t port = 80);

  void begin();

  // true, sobald nach einem erfolgreichen OTA-Update neu gestartet werden soll.
  bool restartDue() const;

 private:
  void registerRoutes();
  void handleGetConfig(AsyncWebServerRequest* request) const;
  void handleGetStatus(AsyncWebServerRequest* request) const;
  void handlePostConfig(AsyncWebServerRequest* request, const JsonVariant& body);
  void handleOtaUpload(AsyncWebServerRequest* request, const String& filename,
                       size_t index, uint8_t* data, size_t length, bool isFinal);
  void handleOtaResult(AsyncWebServerRequest* request);
  bool isAuthenticated(AsyncWebServerRequest* request) const;

  FanControlService& service;
  OtaUpdater& otaUpdater;
  const char* otaUsername;
  const char* otaPassword;
  AsyncWebServer server;
  uint32_t restartAtMs = 0;
};
