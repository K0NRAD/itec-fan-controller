#pragma once

#include <Preferences.h>

#include "IConfigRepository.h"

// Persistiert die Konfiguration im NVS (Non-Volatile Storage) des ESP32.
class NvsConfigStore : public IConfigRepository {
 public:
  FanConfig load() override;
  bool save(const FanConfig& config) override;

 private:
  static constexpr const char* NAMESPACE = "fanctl";

  Preferences preferences;
};
