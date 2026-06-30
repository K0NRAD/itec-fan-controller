#include "FanControlService.h"

#include <Arduino.h>

namespace {
// Intervall der Regelschleife (ms). Schnell genug für Temperaturänderungen,
// ohne den Lüfter unnötig zu modulieren.
constexpr uint32_t CONTROL_INTERVAL_MS = 1000;
}  // namespace

FanControlService::FanControlService(IFanDriver& fanDriver,
                                     ITemperatureSensor& temperatureSensor,
                                     IConfigRepository& configRepository)
    : fanDriver(fanDriver),
      temperatureSensor(temperatureSensor),
      configRepository(configRepository) {}

void FanControlService::begin() {
  config = fanconfig::sanitize(configRepository.load());
  fanDriver.begin();
  if (config.temperatureControlEnabled) {
    temperatureSensor.begin();
  }
  applyControl();
  lastControlMs = millis();
}

void FanControlService::update() {
  if (config.temperatureControlEnabled) {
    temperatureSensor.update();
  }

  const uint32_t now = millis();
  if (now - lastControlMs >= CONTROL_INTERVAL_MS) {
    lastControlMs = now;
    applyControl();
  }
}

const FanConfig& FanControlService::getConfig() const { return config; }

bool FanControlService::updateConfig(const FanConfig& newConfig) {
  const bool wasEnabled = config.temperatureControlEnabled;
  config = fanconfig::sanitize(newConfig);

  // Sensor erst initialisieren, wenn die Regelung neu aktiviert wurde.
  if (config.temperatureControlEnabled && !wasEnabled) {
    temperatureSensor.begin();
  }

  const bool saved = configRepository.save(config);
  applyControl();

  Serial.printf(
      "Config: base=%u%% tempCtrl=%d threshold=%.1f maxTemp=%.1f maxSpeed=%u%% "
      "-> duty=%u%%\n",
      config.baseSpeedPercent, config.temperatureControlEnabled,
      config.thresholdTempC, config.maxTempC, config.maxSpeedPercent,
      fanDriver.getDutyPercent());
  return saved;
}

FanStatus FanControlService::getStatus() const {
  FanStatus status;
  status.dutyPercent = fanDriver.getDutyPercent();
  status.sensorEnabled = config.temperatureControlEnabled;

  if (config.temperatureControlEnabled) {
    float celsius = 0.0f;
    status.temperatureValid = temperatureSensor.readCelsius(celsius);
    status.temperatureC = status.temperatureValid ? celsius : 0.0f;
  }
  return status;
}

void FanControlService::applyControl() {
  float celsius = 0.0f;
  bool temperatureValid = false;
  if (config.temperatureControlEnabled) {
    temperatureValid = temperatureSensor.readCelsius(celsius);
  }

  const uint8_t speedPercent =
      selectStrategy().computeSpeedPercent(config, celsius, temperatureValid);
  fanDriver.setDutyPercent(speedPercent);
}

const ISpeedStrategy& FanControlService::selectStrategy() const {
  if (config.temperatureControlEnabled) {
    return temperatureStrategy;
  }
  return fixedStrategy;
}
