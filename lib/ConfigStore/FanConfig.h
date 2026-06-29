#pragma once

#include <algorithm>
#include <cstdint>

// Persistente Einstellungen der Lüftersteuerung.
struct FanConfig {
  uint8_t baseSpeedPercent = 30;          // Basis-/Fixdrehzahl in %
  bool temperatureControlEnabled = false; // Temperaturregelung aktiv?
  float thresholdTempC = 35.0f;           // ab hier wird hochgeregelt
  float maxTempC = 60.0f;                 // hier wird maxSpeedPercent erreicht
  uint8_t maxSpeedPercent = 100;          // Maximaldrehzahl bei maxTempC in %
};

namespace fanconfig {

// Erlaubter Temperaturbereich (DS18B20).
constexpr float MIN_TEMP_C = -55.0f;
constexpr float MAX_TEMP_C = 125.0f;
constexpr float MIN_TEMP_SPAN_C = 1.0f;  // thresholdTempC < maxTempC erzwingen

// Begrenzt einen Wert auf [low, high].
template <typename T>
inline T clampValue(T value, T low, T high) {
  return std::max(low, std::min(value, high));
}

// Begrenzt alle Felder auf gültige Wertebereiche und stellt konsistente
// Invarianten her (maxSpeed >= baseSpeed, maxTemp > thresholdTemp).
inline FanConfig sanitize(FanConfig config) {
  config.baseSpeedPercent = std::min<uint8_t>(config.baseSpeedPercent, 100);
  config.maxSpeedPercent = std::min<uint8_t>(config.maxSpeedPercent, 100);
  config.maxSpeedPercent =
      std::max(config.maxSpeedPercent, config.baseSpeedPercent);

  config.thresholdTempC = clampValue(config.thresholdTempC, MIN_TEMP_C, MAX_TEMP_C);
  config.maxTempC = clampValue(config.maxTempC, MIN_TEMP_C, MAX_TEMP_C);
  if (config.maxTempC < config.thresholdTempC + MIN_TEMP_SPAN_C) {
    config.maxTempC = config.thresholdTempC + MIN_TEMP_SPAN_C;
  }
  return config;
}

}  // namespace fanconfig
