#pragma once

#include "ISpeedStrategy.h"

// Konstante Drehzahl: immer die konfigurierte Basis-Drehzahl.
class FixedSpeedStrategy : public ISpeedStrategy {
 public:
  uint8_t computeSpeedPercent(const FanConfig& config, float /*temperatureC*/,
                              bool /*temperatureValid*/) const override {
    return config.baseSpeedPercent;
  }
};
