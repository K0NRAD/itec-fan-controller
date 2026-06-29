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

## Release / Factory-Image

Der GitHub-Release-Workflow veröffentlicht neben den Einzeldateien
(`firmware-<version>.bin`, `bootloader.bin`, `partitions.bin`, `littlefs.bin`)
auch ein kombiniertes **`factory-<version>.bin`** (Bootloader + Partitionstabelle
+ OTA-Data + App + LittleFS in einer Datei). Ein leeres Board lässt sich damit in
einem Schritt komplett bespielen:

```bash
esptool --chip esp32 write_flash 0x0 factory-<version>.bin
```

### Factory-Image lokal erzeugen

Das gleiche Image kann auch lokal gebaut werden (Offsets: klassischer ESP32,
4-MB-Default-Partition):

```bash
pip install "esptool==4.8.1"

# Firmware und Dateisystem bauen
pio run -e esp32devkitc-v4
pio run -e esp32devkitc-v4 -t buildfs

# Pfade ermitteln und Image mergen
BUILD=.pio/build/esp32devkitc-v4
BOOT_APP0=$(find ~/.platformio/packages -name boot_app0.bin | head -n 1)

python -m esptool --chip esp32 merge_bin -o factory.bin \
  --flash_mode dio --flash_freq 40m --flash_size 4MB \
  0x1000   "$BUILD/bootloader.bin" \
  0x8000   "$BUILD/partitions.bin" \
  0xe000   "$BOOT_APP0" \
  0x10000  "$BUILD/firmware.bin" \
  0x290000 "$BUILD/littlefs.bin"

# Auf ein leeres Board flashen
python -m esptool --chip esp32 write_flash 0x0 factory.bin
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
