#pragma once

#include "ISpeedStrategy.h"

// Temperaturabhängige Regelung:
//   temperatureC <= thresholdTempC      -> baseSpeedPercent
//   temperatureC >= maxTempC            -> maxSpeedPercent
//   dazwischen                          -> lineare Interpolation
// Bei ungültigem Messwert wird sicherheitshalber die Basis-Drehzahl gewählt.
class TemperatureCurveStrategy : public ISpeedStrategy {
 public:
  uint8_t computeSpeedPercent(const FanConfig& config, float temperatureC,
                              bool temperatureValid) const override;
};
