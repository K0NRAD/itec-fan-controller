#include "NvsConfigStore.h"

namespace {
constexpr const char* KEY_BASE_SPEED = "baseSpeed";
constexpr const char* KEY_TEMP_ENABLED = "tempEnabled";
constexpr const char* KEY_THRESHOLD = "threshold";
constexpr const char* KEY_MAX_TEMP = "maxTemp";
constexpr const char* KEY_MAX_SPEED = "maxSpeed";
}  // namespace

FanConfig NvsConfigStore::load() {
  const FanConfig defaults;
  FanConfig config;

  preferences.begin(NAMESPACE, /*readOnly=*/true);
  config.baseSpeedPercent =
      preferences.getUChar(KEY_BASE_SPEED, defaults.baseSpeedPercent);
  config.temperatureControlEnabled =
      preferences.getBool(KEY_TEMP_ENABLED, defaults.temperatureControlEnabled);
  config.thresholdTempC =
      preferences.getFloat(KEY_THRESHOLD, defaults.thresholdTempC);
  config.maxTempC = preferences.getFloat(KEY_MAX_TEMP, defaults.maxTempC);
  config.maxSpeedPercent =
      preferences.getUChar(KEY_MAX_SPEED, defaults.maxSpeedPercent);
  preferences.end();

  return fanconfig::sanitize(config);
}

bool NvsConfigStore::save(const FanConfig& config) {
  const FanConfig sanitized = fanconfig::sanitize(config);

  if (!preferences.begin(NAMESPACE, /*readOnly=*/false)) {
    return false;
  }
  preferences.putUChar(KEY_BASE_SPEED, sanitized.baseSpeedPercent);
  preferences.putBool(KEY_TEMP_ENABLED, sanitized.temperatureControlEnabled);
  preferences.putFloat(KEY_THRESHOLD, sanitized.thresholdTempC);
  preferences.putFloat(KEY_MAX_TEMP, sanitized.maxTempC);
  preferences.putUChar(KEY_MAX_SPEED, sanitized.maxSpeedPercent);
  preferences.end();

  return true;
}
