#pragma once

#include "IFanDriver.h"

// Lüfter-Treiber auf Basis der ESP32-LEDC-Hardware-PWM (Arduino-Core 3.x API).
// Erzeugt das 25-kHz-PWM-Signal zur Drehzahlsteuerung.
class FanDriver : public IFanDriver {
 public:
  FanDriver(uint8_t pwmPin, uint32_t pwmFrequencyHz, uint8_t resolutionBits);

  void begin() override;
  void setDutyPercent(uint8_t percent) override;
  uint8_t getDutyPercent() const override;

 private:
  uint16_t maxDutyValue() const;

  const uint8_t pwmPin;
  const uint32_t pwmFrequencyHz;
  const uint8_t resolutionBits;

  uint8_t currentDutyPercent = 0;
};
