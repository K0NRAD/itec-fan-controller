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
+ OTA-Data + App + LittleFS in einer Datei).

### Aus einem Release flashen

Voraussetzung: `pip install esptool` (oder `python -m esptool ...`). Port anpassen
(macOS z. B. `/dev/cu.usbserial-XXXX`, Linux `/dev/ttyUSB0`, Windows `COMx`); ohne
`-p` versucht esptool die Auto-Erkennung. Optional `--baud 921600` für mehr Tempo.

**Weg 1 (empfohlen): Factory-Image – eine Datei, ein Befehl.** Ideal für ein leeres
Board und für Web-Flasher (z. B. ESP Web Tools), dort bei Offset `0x0` laden:

```bash
esptool --chip esp32 -p /dev/cu.usbserial-14120 write_flash 0x0 factory-<version>.bin
```

**Weg 2: Einzeldateien an ihre Offsets** (z. B. um nur Teile zu aktualisieren):

```bash
esptool --chip esp32 -p /dev/cu.usbserial-14120 write_flash \
  0x1000   bootloader.bin \
  0x8000   partitions.bin \
  0x10000  firmware-<version>.bin \
  0x290000 littlefs.bin
```

- Nur Anwendung aktualisieren: `... write_flash 0x10000 firmware-<version>.bin`
- Nur Webinterface aktualisieren: `... write_flash 0x290000 littlefs.bin`

Hinweise:

- `firmware-<version>.elf` ist **nicht** zum Flashen, sondern die Debug-/Symboldatei
  (z. B. für Stacktrace-Decoding).
- `boot_app0.bin` liegt dem Release nicht bei. Bei Weg 2 unkritisch: Bei leerem
  OTA-Data bootet der Bootloader ohnehin App0. Im Factory-Image ist es enthalten.

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

## OTA-Update (über das Webinterface)

Nach dem ersten Flashen lässt sich die Firmware **drahtlos** über die Weboberfläche
aktualisieren – kein USB-Kabel mehr nötig. Das Default-Partitionslayout besitzt zwei
App-Slots; das Update wird in den inaktiven Slot geschrieben und nach erfolgreicher
Prüfung per Neustart aktiviert.

Ablauf:

1. In der Weboberfläche zur Karte **„Firmware-Update (OTA)"** scrollen.
2. Datei wählen:
   - `firmware-<version>.bin` → aktualisiert die **Anwendung**
   - `littlefs.bin` → aktualisiert das **Webinterface** (Dateisystem)
   - Die Erkennung erfolgt über den Dateinamen (Name mit `littlefs`/`spiffs` = Dateisystem).
3. **OTA-Passwort** eingeben und „Hochladen & flashen" klicken.
4. Nach erfolgreichem Upload startet das Gerät automatisch neu.

Zugangsdaten (Basic-Auth): Standard **`admin` / `fancontrol-ota`**. Über
`include/secrets.h` (`OTA_USERNAME` / `OTA_PASSWORD`) überschreibbar.

Per Kommandozeile (z. B. im AP-Modus unter `192.168.4.1`):

```bash
curl -u admin:fancontrol-ota -F "firmware=@firmware-<version>.bin" http://192.168.4.1/update
```

> Sicherheitshinweis: Die Authentifizierung erfolgt per HTTP Basic-Auth über eine
> unverschlüsselte Verbindung. Das ist für ein lokales Netz üblich; das OTA-Passwort
> sollte dennoch geändert werden und das Gerät nicht offen erreichbar sein.

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
| `WebInterface`       | HTTP-/REST-API, Webfrontend und OTA-Upload-Route         |
| `OtaUpdater`         | OTA-Update (Firmware/Dateisystem) über die Update-API    |
