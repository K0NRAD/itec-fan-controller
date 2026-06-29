#pragma once

#include "IFanDriver.h"

// Lüfter-Treiber auf Basis der ESP32-LEDC-Hardware-PWM (Arduino-Core 3.x API).
// Erzeugt das 25-kHz-PWM-Signal und misst die Drehzahl über den Tacho-Eingang
// (Interrupt, fallende Flanke).
//
// Hinweis: Aufgrund der Interrupt-Service-Routine ist nur EINE Instanz zulässig
// (ein Lüfter). Das entspricht der Anforderung (YAGNI).
class LedcFanDriver : public IFanDriver {
 public:
  LedcFanDriver(uint8_t pwmPin, uint8_t tachPin, uint32_t pwmFrequencyHz,
                uint8_t resolutionBits, uint8_t tachPulsesPerRev);

  void begin() override;
  void update() override;
  void setDutyPercent(uint8_t percent) override;
  uint8_t getDutyPercent() const override;
  uint16_t getRpm() const override;

 private:
  uint16_t maxDutyValue() const;

  const uint8_t pwmPin;
  const uint8_t tachPin;
  const uint32_t pwmFrequencyHz;
  const uint8_t resolutionBits;
  const uint8_t tachPulsesPerRev;

  uint8_t currentDutyPercent = 0;
  uint16_t currentRpm = 0;
  uint32_t lastSampleMs = 0;
};
