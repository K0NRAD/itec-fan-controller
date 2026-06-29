#pragma once

#include <cstdint>

// Abstraktion des Lüfter-Treibers (DIP). Kapselt PWM-Ausgabe und
// Drehzahlmessung, sodass die Steuerlogik hardwareunabhängig bleibt.
class IFanDriver {
 public:
  virtual ~IFanDriver() = default;

  // Initialisiert PWM-Peripherie und Tacho-Eingang.
  virtual void begin() = 0;

  // Periodisch aufzurufen (loop): aktualisiert die Drehzahlmessung.
  virtual void update() = 0;

  // Setzt das Tastverhältnis in Prozent (0..100).
  virtual void setDutyPercent(uint8_t percent) = 0;

  // Aktuell gesetztes Tastverhältnis in Prozent.
  virtual uint8_t getDutyPercent() const = 0;

  // Zuletzt gemessene Drehzahl in U/min.
  virtual uint16_t getRpm() const = 0;
};
