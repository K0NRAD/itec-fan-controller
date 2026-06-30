#pragma once

#include <cstdint>

// Abstraktion des Lüfter-Treibers (DIP). Kapselt die PWM-Ausgabe, sodass die
// Steuerlogik hardwareunabhängig bleibt.
class IFanDriver {
 public:
  virtual ~IFanDriver() = default;

  // Initialisiert die PWM-Peripherie.
  virtual void begin() = 0;

  // Setzt das Tastverhältnis in Prozent (0..100).
  virtual void setDutyPercent(uint8_t percent) = 0;

  // Aktuell gesetztes Tastverhältnis in Prozent.
  virtual uint8_t getDutyPercent() const = 0;
};
