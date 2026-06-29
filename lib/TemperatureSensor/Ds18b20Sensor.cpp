#include "Ds18b20Sensor.h"

#include <Arduino.h>

Ds18b20Sensor::Ds18b20Sensor(uint8_t oneWirePin)
    : oneWire(oneWirePin), sensors(&oneWire) {}

bool Ds18b20Sensor::begin() {
  sensors.begin();
  sensors.setResolution(RESOLUTION_BITS);
  // Nicht blockieren: requestTemperatures() kehrt sofort zurück.
  sensors.setWaitForConversion(false);
  conversionDelayMs = sensors.millisToWaitForConversion(RESOLUTION_BITS);

  deviceConnected = sensors.getDeviceCount() > 0;
  conversionPending = false;
  hasValidReading = false;
  return deviceConnected;
}

void Ds18b20Sensor::update() {
  if (!conversionPending) {
    if (sensors.getDeviceCount() == 0) {
      deviceConnected = false;
      return;
    }
    deviceConnected = true;
    sensors.requestTemperatures();
    conversionRequestedMs = millis();
    conversionPending = true;
    return;
  }

  if (millis() - conversionRequestedMs < conversionDelayMs) {
    return;
  }

  const float celsius = sensors.getTempCByIndex(0);
  conversionPending = false;

  if (celsius == DEVICE_DISCONNECTED_C) {
    deviceConnected = false;
    return;
  }
  deviceConnected = true;
  lastValidCelsius = celsius;
  hasValidReading = true;
}

bool Ds18b20Sensor::readCelsius(float& outCelsius) const {
  if (!hasValidReading || !deviceConnected) {
    return false;
  }
  outCelsius = lastValidCelsius;
  return true;
}

bool Ds18b20Sensor::isConnected() const { return deviceConnected; }
