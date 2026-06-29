#pragma once

// Abstraktion eines Temperatursensors (DIP/ISP). Ermöglicht Austausch der
// konkreten Sensorhardware und Mocking in Tests.
class ITemperatureSensor {
 public:
  virtual ~ITemperatureSensor() = default;

  // Initialisiert den Sensor. Liefert true, wenn ein Sensor erkannt wurde.
  virtual bool begin() = 0;

  // Periodisch aufzurufen (loop): treibt die nicht-blockierende Messung an.
  virtual void update() = 0;

  // Letzter gültiger Messwert. Liefert false, wenn (noch) kein gültiger Wert
  // vorliegt bzw. der Sensor getrennt ist.
  virtual bool readCelsius(float& outCelsius) const = 0;

  // true, wenn aktuell ein Sensor am Bus erkannt wird.
  virtual bool isConnected() const = 0;
};
