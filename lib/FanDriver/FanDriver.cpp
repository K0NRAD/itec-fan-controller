#include "FanDriver.h"

#include <Arduino.h>

FanDriver::FanDriver(uint8_t pwmPin, uint32_t pwmFrequencyHz,
                     uint8_t resolutionBits)
    : pwmPin(pwmPin),
      pwmFrequencyHz(pwmFrequencyHz),
      resolutionBits(resolutionBits) {}

uint16_t FanDriver::maxDutyValue() const {
  return static_cast<uint16_t>((1u << resolutionBits) - 1u);
}

void FanDriver::begin() {
  ledcAttach(pwmPin, pwmFrequencyHz, resolutionBits);
  setDutyPercent(0);
}

void FanDriver::setDutyPercent(uint8_t percent) {
  currentDutyPercent = percent > 100 ? 100 : percent;
  const uint32_t duty =
      (static_cast<uint32_t>(currentDutyPercent) * maxDutyValue()) / 100u;
  ledcWrite(pwmPin, duty);
}

uint8_t FanDriver::getDutyPercent() const { return currentDutyPercent; }
