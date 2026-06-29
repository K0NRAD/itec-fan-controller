#include "FanDriver.h"

#include <Arduino.h>

namespace {
// Tacho-Impulszähler. Wird in der ISR erhöht und im Hauptkontext ausgewertet.
// Nur eine Lüfter-Instanz ist zulässig (siehe Header).
volatile uint32_t tachPulseCount = 0;

void IRAM_ATTR onTachPulse() { tachPulseCount = tachPulseCount + 1; }

// Mindestabstand zwischen zwei Drehzahlmessungen (ms).
constexpr uint32_t RPM_SAMPLE_INTERVAL_MS = 1000;
constexpr uint32_t MILLIS_PER_MINUTE = 60000;
}  // namespace

FanDriver::FanDriver(uint8_t pwmPin, uint8_t tachPin,
                             uint32_t pwmFrequencyHz, uint8_t resolutionBits,
                             uint8_t tachPulsesPerRev)
    : pwmPin(pwmPin),
      tachPin(tachPin),
      pwmFrequencyHz(pwmFrequencyHz),
      resolutionBits(resolutionBits),
      tachPulsesPerRev(tachPulsesPerRev) {}

uint16_t FanDriver::maxDutyValue() const {
  return static_cast<uint16_t>((1u << resolutionBits) - 1u);
}

void FanDriver::begin() {
  ledcAttach(pwmPin, pwmFrequencyHz, resolutionBits);
  setDutyPercent(0);

  pinMode(tachPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tachPin), onTachPulse, FALLING);
  lastSampleMs = millis();
}

void FanDriver::setDutyPercent(uint8_t percent) {
  currentDutyPercent = percent > 100 ? 100 : percent;
  const uint32_t duty =
      (static_cast<uint32_t>(currentDutyPercent) * maxDutyValue()) / 100u;
  ledcWrite(pwmPin, duty);
}

uint8_t FanDriver::getDutyPercent() const { return currentDutyPercent; }

uint16_t FanDriver::getRpm() const { return currentRpm; }

void FanDriver::update() {
  const uint32_t now = millis();
  const uint32_t elapsedMs = now - lastSampleMs;
  if (elapsedMs < RPM_SAMPLE_INTERVAL_MS) {
    return;
  }

  noInterrupts();
  const uint32_t pulses = tachPulseCount;
  tachPulseCount = 0;
  interrupts();

  // RPM = (Impulse / Impulse-pro-Umdrehung) / (Zeit in Minuten)
  const uint32_t revolutions = pulses / tachPulsesPerRev;
  currentRpm = static_cast<uint16_t>((revolutions * MILLIS_PER_MINUTE) /
                                     elapsedMs);
  lastSampleMs = now;
}
