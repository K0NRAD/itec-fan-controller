#include "TemperatureCurveStrategy.h"

uint8_t TemperatureCurveStrategy::computeSpeedPercent(
    const FanConfig& config, float temperatureC, bool temperatureValid) const {
  // Fail-safe: ohne gültigen Messwert auf Basis-Drehzahl zurückfallen.
  if (!temperatureValid) {
    return config.baseSpeedPercent;
  }
  if (temperatureC <= config.thresholdTempC) {
    return config.baseSpeedPercent;
  }
  if (temperatureC >= config.maxTempC) {
    return config.maxSpeedPercent;
  }

  // sanitize() garantiert maxTempC > thresholdTempC -> Division ist sicher.
  const float span = config.maxTempC - config.thresholdTempC;
  const float ratio = (temperatureC - config.thresholdTempC) / span;
  const float speedRange = static_cast<float>(config.maxSpeedPercent) -
                           static_cast<float>(config.baseSpeedPercent);
  const float speed = static_cast<float>(config.baseSpeedPercent) +
                      ratio * speedRange;
  return static_cast<uint8_t>(speed + 0.5f);
}
