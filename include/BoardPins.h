#pragma once

#include <cstdint>

// Zentrale Hardware-Konfiguration (GPIO-Belegung und PWM-Parameter).
// Siehe docs/wiring-diagram.md und docs/hardware-datasheets.md.
namespace board {

// GPIO-Belegung ESP32 DevKitC v4
constexpr uint8_t FAN_PWM_PIN = 25;     // LEDC-Ausgang -> Lüfter Pin 4 (blau)
constexpr uint8_t FAN_TACH_PIN = 26;    // Interrupt-Eingang <- Lüfter Pin 3 (grün)
constexpr uint8_t TEMP_SENSOR_PIN = 4;  // 1-Wire <-> DS18B20 (4,7 kΩ Pull-up)

// PWM-Parameter nach Intel-4-Wire-Spec
constexpr uint32_t FAN_PWM_FREQUENCY_HZ = 25000;  // 25 kHz
constexpr uint8_t FAN_PWM_RESOLUTION_BITS = 8;    // 0..255 Stufen
constexpr uint8_t FAN_TACH_PULSES_PER_REV = 2;    // Noctua: 2 Impulse/Umdrehung

}  // namespace board
