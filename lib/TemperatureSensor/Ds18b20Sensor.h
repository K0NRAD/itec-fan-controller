#pragma once

#include <DallasTemperature.h>
#include <OneWire.h>

#include "ITemperatureSensor.h"

// DS18B20-Implementierung. Die Konvertierung läuft nicht-blockierend: update()
// stößt eine Messung an und liest sie nach Ablauf der Konvertierungszeit aus,
// ohne loop() zu blockieren.
class Ds18b20Sensor : public ITemperatureSensor {
 public:
  explicit Ds18b20Sensor(uint8_t oneWirePin);

  bool begin() override;
  void update() override;
  bool readCelsius(float& outCelsius) const override;

 private:
  static constexpr uint8_t RESOLUTION_BITS = 12;

  OneWire oneWire;
  DallasTemperature sensors;

  bool conversionPending = false;
  uint32_t conversionRequestedMs = 0;
  uint32_t conversionDelayMs = 750;  // 12-Bit-Konvertierungszeit
  bool deviceConnected = false;
  bool hasValidReading = false;
  float lastValidCelsius = 0.0f;
};
