#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "BoardPins.h"
#include "Ds18b20Sensor.h"
#include "FanControlService.h"
#include "LedcFanDriver.h"
#include "NvsConfigStore.h"
#include "WebInterface.h"

// Optionale WLAN-Zugangsdaten (include/secrets.h, per .gitignore ausgeschlossen).
#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

namespace {
constexpr const char* ACCESS_POINT_SSID = "itec-fan-controller";
constexpr const char* ACCESS_POINT_PASSWORD = "fancontrol";
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;

// Komposition (Dependency Injection) der Module – einzige Stelle mit konkreten
// Implementierungen.
NvsConfigStore configStore;
LedcFanDriver fanDriver(board::FAN_PWM_PIN, board::FAN_TACH_PIN,
                        board::FAN_PWM_FREQUENCY_HZ,
                        board::FAN_PWM_RESOLUTION_BITS,
                        board::FAN_TACH_PULSES_PER_REV);
Ds18b20Sensor temperatureSensor(board::TEMP_SENSOR_PIN);
FanControlService fanControlService(fanDriver, temperatureSensor, configStore);
WebInterface webInterface(fanControlService);

// Verbindet sich als Station; bei Fehlschlag wird ein Access Point gestartet,
// damit das Webinterface stets erreichbar ist.
void initNetwork() {
  const char* ssid = WIFI_SSID;
  if (ssid[0] != '\0') {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    const uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startMs < WIFI_CONNECT_TIMEOUT_MS) {
      delay(250);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("WLAN verbunden, IP: %s\n", WiFi.localIP().toString().c_str());
      return;
    }
    Serial.println("WLAN-Verbindung fehlgeschlagen, starte Access Point.");
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ACCESS_POINT_SSID, ACCESS_POINT_PASSWORD);
  Serial.printf("Access Point '%s', IP: %s\n", ACCESS_POINT_SSID,
                WiFi.softAPIP().toString().c_str());
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nitec-fan-controller startet ...");

  if (!LittleFS.begin(/*formatOnFail=*/true)) {
    Serial.println("LittleFS-Mount fehlgeschlagen.");
  }

  initNetwork();
  fanControlService.begin();
  webInterface.begin();

  Serial.println("Bereit.");
}

void loop() {
  fanControlService.update();
  delay(10);
}
