#pragma once

#include "FanConfig.h"
#include "FanStatus.h"
#include "FixedSpeedStrategy.h"
#include "IConfigRepository.h"
#include "IFanDriver.h"
#include "ITemperatureSensor.h"
#include "TemperatureCurveStrategy.h"

// Orchestriert Sensor, Regelstrategie und Lüfter-Treiber. Hängt ausschließlich
// von Abstraktionen ab (DIP) und besitzt die einzige Verantwortung, aus
// Konfiguration + Messwert das Tastverhältnis abzuleiten und anzuwenden.
class FanControlService {
 public:
  FanControlService(IFanDriver& fanDriver,
                    ITemperatureSensor& temperatureSensor,
                    IConfigRepository& configRepository);

  // Lädt Konfiguration, initialisiert Treiber/Sensor und wendet die Regelung an.
  void begin();

  // Periodisch im loop() aufzurufen.
  void update();

  const FanConfig& getConfig() const;

  // Validiert, persistiert und übernimmt eine neue Konfiguration.
  // Liefert true, wenn das Speichern erfolgreich war.
  bool updateConfig(const FanConfig& newConfig);

  FanStatus getStatus() const;

 private:
  void applyControl();
  const ISpeedStrategy& selectStrategy() const;

  IFanDriver& fanDriver;
  ITemperatureSensor& temperatureSensor;
  IConfigRepository& configRepository;

  FixedSpeedStrategy fixedStrategy;
  TemperatureCurveStrategy temperatureStrategy;

  FanConfig config;
  uint32_t lastControlMs = 0;
};
