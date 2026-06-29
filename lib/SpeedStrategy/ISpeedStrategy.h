#pragma once

#include <cstdint>

#include "FanConfig.h"

// Strategy-Abstraktion zur Bestimmung der Solldrehzahl (OCP). Neue Regelarten
// können ohne Änderung der bestehenden Logik ergänzt werden.
class ISpeedStrategy {
 public:
  virtual ~ISpeedStrategy() = default;

  // Berechnet das Soll-Tastverhältnis in Prozent (0..100).
  // temperatureValid signalisiert, ob temperatureC ein gültiger Messwert ist.
  virtual uint8_t computeSpeedPercent(const FanConfig& config,
                                      float temperatureC,
                                      bool temperatureValid) const = 0;
};
