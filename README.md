# itec-fan-controller

Firmware zur PWM-Steuerung eines 4-Pin-PC-Lüfters (Noctua NF-F12 industrialPPC) mit
einem ESP32 DevKitC v4. Die Lüfterdrehzahl wird über ein Webinterface eingestellt.
Optional kann ein DS18B20-Temperatursensor aktiviert werden, der die Drehzahl
temperaturabhängig regelt (Basis-Drehzahl + Anhebung bei Überschreiten einer Schwelle).

## Features

- PWM-Lüftersteuerung mit 25 kHz (Intel-4-Wire-Spec) über die LEDC-Hardware des ESP32
- Drehzahlmessung über den Tacho-Ausgang (RPM)
- Webinterface zur Konfiguration (fixe Drehzahl oder Temperatur-Regelung)
- Aktivier-/Deaktivierbarer DS18B20-Temperatursensor
- Persistente Speicherung der Einstellungen (NVS)
- CI/CD: automatische Versionierung, Tagging und GitHub-Release der Firmware-Binaries

## Hardware

Siehe [docs/hardware-datasheets.md](docs/hardware-datasheets.md) und das
Anschluss-Schema in [docs/wiring-diagram.md](docs/wiring-diagram.md).

## Build

```bash
pio run                 # Firmware bauen
pio run -t upload       # Firmware flashen
pio run -t uploadfs     # Webinterface (data/) ins LittleFS flashen
pio device monitor      # Serielle Ausgabe
```

## Architektur

Die Firmware folgt den SOLID-Prinzipien. Jede Verantwortlichkeit liegt in einer
eigenen Bibliothek unter `lib/`:

| Modul                | Verantwortung                                            |
| -------------------- | -------------------------------------------------------- |
| `ConfigStore`        | Persistenz der Einstellungen (Repository-Pattern, NVS)   |
| `TemperatureSensor`  | Abstraktion (`ITemperatureSensor`) + DS18B20-Impl.       |
| `FanDriver`          | PWM-Ausgabe + Tacho-Messung (`IFanDriver` + LEDC-Impl.)  |
| `SpeedStrategy`      | Strategy-Pattern: feste Drehzahl vs. Temperaturkurve     |
| `FanControlService`  | Orchestrierung von Sensor, Strategie und Treiber         |
| `WebInterface`       | HTTP-/REST-API und Auslieferung des Webfrontends         |
