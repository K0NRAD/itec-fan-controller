#include "WebInterface.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <LittleFS.h>

namespace {
constexpr const char* MIME_JSON = "application/json";

// Schlüsselnamen der REST-API (zentral, um Tippfehler zu vermeiden).
constexpr const char* KEY_BASE_SPEED = "baseSpeedPercent";
constexpr const char* KEY_TEMP_ENABLED = "temperatureControlEnabled";
constexpr const char* KEY_THRESHOLD = "thresholdTempC";
constexpr const char* KEY_MAX_TEMP = "maxTempC";
constexpr const char* KEY_MAX_SPEED = "maxSpeedPercent";

void serializeConfig(const FanConfig& config, JsonObject target) {
  target[KEY_BASE_SPEED] = config.baseSpeedPercent;
  target[KEY_TEMP_ENABLED] = config.temperatureControlEnabled;
  target[KEY_THRESHOLD] = config.thresholdTempC;
  target[KEY_MAX_TEMP] = config.maxTempC;
  target[KEY_MAX_SPEED] = config.maxSpeedPercent;
}
}  // namespace

WebInterface::WebInterface(FanControlService& service, OtaUpdater& otaUpdater,
                           const char* otaUsername, const char* otaPassword,
                           uint16_t port)
    : service(service),
      otaUpdater(otaUpdater),
      otaUsername(otaUsername),
      otaPassword(otaPassword),
      server(port) {}

void WebInterface::begin() {
  registerRoutes();
  server.begin();
}

bool WebInterface::restartDue() const {
  return restartAtMs != 0 && millis() >= restartAtMs;
}

void WebInterface::registerRoutes() {
  server.on("/api/config", HTTP_GET,
            [this](AsyncWebServerRequest* request) { handleGetConfig(request); });

  server.on("/api/status", HTTP_GET,
            [this](AsyncWebServerRequest* request) { handleGetStatus(request); });

  auto* configWriteHandler = new AsyncCallbackJsonWebHandler(
      "/api/config",
      [this](AsyncWebServerRequest* request, JsonVariant& body) {
        handlePostConfig(request, body);
      });
  configWriteHandler->setMethod(HTTP_POST);
  server.addHandler(configWriteHandler);

  // Passwortgeschützter OTA-Upload (Firmware oder Dateisystem-Image).
  server.on(
      "/update", HTTP_POST,
      [this](AsyncWebServerRequest* request) { handleOtaResult(request); },
      [this](AsyncWebServerRequest* request, const String& filename,
             size_t index, uint8_t* data, size_t length, bool isFinal) {
        handleOtaUpload(request, filename, index, data, length, isFinal);
      });

  // Statische Auslieferung des Webfrontends aus dem LittleFS.
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not Found");
  });
}

void WebInterface::handleGetConfig(AsyncWebServerRequest* request) const {
  JsonDocument doc;
  serializeConfig(service.getConfig(), doc.to<JsonObject>());

  String payload;
  serializeJson(doc, payload);
  request->send(200, MIME_JSON, payload);
}

void WebInterface::handleGetStatus(AsyncWebServerRequest* request) const {
  const FanStatus status = service.getStatus();

  JsonDocument doc;
  doc["dutyPercent"] = status.dutyPercent;
  doc["rpm"] = status.rpm;
  doc["sensorEnabled"] = status.sensorEnabled;
  doc["temperatureValid"] = status.temperatureValid;
  if (status.temperatureValid) {
    doc["temperatureC"] = status.temperatureC;
  }

  String payload;
  serializeJson(doc, payload);
  request->send(200, MIME_JSON, payload);
}

void WebInterface::handlePostConfig(AsyncWebServerRequest* request,
                                    const JsonVariant& body) {
  if (!body.is<JsonObject>()) {
    request->send(400, MIME_JSON, "{\"error\":\"invalid JSON body\"}");
    return;
  }
  const JsonObjectConst input = body.as<JsonObjectConst>();

  // Bestehende Werte übernehmen und nur übergebene Felder überschreiben.
  FanConfig updated = service.getConfig();
  if (input[KEY_BASE_SPEED].is<uint8_t>()) {
    updated.baseSpeedPercent = input[KEY_BASE_SPEED].as<uint8_t>();
  }
  if (input[KEY_TEMP_ENABLED].is<bool>()) {
    updated.temperatureControlEnabled = input[KEY_TEMP_ENABLED].as<bool>();
  }
  if (input[KEY_THRESHOLD].is<float>()) {
    updated.thresholdTempC = input[KEY_THRESHOLD].as<float>();
  }
  if (input[KEY_MAX_TEMP].is<float>()) {
    updated.maxTempC = input[KEY_MAX_TEMP].as<float>();
  }
  if (input[KEY_MAX_SPEED].is<uint8_t>()) {
    updated.maxSpeedPercent = input[KEY_MAX_SPEED].as<uint8_t>();
  }

  const bool saved = service.updateConfig(updated);

  // Wirksame (sanitisierte) Konfiguration zurückgeben.
  JsonDocument doc;
  doc["saved"] = saved;
  serializeConfig(service.getConfig(), doc["config"].to<JsonObject>());

  String payload;
  serializeJson(doc, payload);
  request->send(saved ? 200 : 500, MIME_JSON, payload);
}

bool WebInterface::isAuthenticated(AsyncWebServerRequest* request) const {
  return request->authenticate(otaUsername, otaPassword);
}

void WebInterface::handleOtaUpload(AsyncWebServerRequest* request,
                                   const String& filename, size_t index,
                                   uint8_t* data, size_t length, bool isFinal) {
  // Ohne gültige Authentifizierung werden keine Daten in den Flash geschrieben.
  if (!isAuthenticated(request)) {
    return;
  }
  // Dateiname entscheidet, ob Firmware oder Dateisystem-Image aktualisiert wird.
  const bool filesystem =
      filename.indexOf("littlefs") >= 0 || filename.indexOf("spiffs") >= 0;
  otaUpdater.processChunk(data, length, index, isFinal, filesystem);
}

void WebInterface::handleOtaResult(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request)) {
    return request->requestAuthentication();
  }

  const bool ok = otaUpdater.isFinished() && !otaUpdater.hasError();
  String body = ok ? String("{\"ok\":true}")
                   : String("{\"ok\":false,\"error\":\"") +
                         otaUpdater.errorMessage() + "\"}";

  AsyncWebServerResponse* response =
      request->beginResponse(ok ? 200 : 500, MIME_JSON, body);
  response->addHeader("Connection", "close");
  request->send(response);

  // Neustart erst nach Auslieferung der Antwort (im loop() ausgewertet).
  if (ok) {
    restartAtMs = millis() + 1500;
  }
}
