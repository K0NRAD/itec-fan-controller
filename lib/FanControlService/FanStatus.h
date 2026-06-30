#pragma once

#include <cstdint>

// Momentaufnahme des Betriebszustands (für das Webinterface).
struct FanStatus {
  uint8_t dutyPercent = 0;       // aktuelles Tastverhältnis in %
  float temperatureC = 0.0f;     // aktuelle Temperatur (falls gültig)
  bool temperatureValid = false; // ist temperatureC gültig?
  bool sensorEnabled = false;    // ist die Temperaturregelung aktiv?
};
