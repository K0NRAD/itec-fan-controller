# Hardware & Datenblatt-Zusammenfassung

Zusammenfassung der relevanten Spezifikationen, die der Firmware- und Schaltungs-
Auslegung zugrunde liegen. Quellen siehe Abschnitt "Referenzen".

## 1. Lüfter: Noctua NF-F12 industrialPPC PWM

Vierpoliger PWM-Lüfter nach Intel-4-Wire-Spezifikation. Es existieren die Varianten
`-2000 PWM` und `-3000 PWM` (sowie IP-Schutz-Versionen). Maßgeblich für die
Netzteil-Auslegung ist die leistungsstärkere `-3000`-Variante.

| Parameter                | NF-F12 iPPC-2000 PWM | NF-F12 iPPC-3000 PWM |
| ------------------------ | -------------------- | -------------------- |
| Nennspannung (Power)     | 12 V DC              | 12 V DC              |
| Max. Drehzahl            | 2000 U/min (±10 %)   | 3000 U/min (±10 %)   |
| Min. Drehzahl (PWM)      | ca. 400–600 U/min    | ca. 750 U/min        |
| Max. Stromaufnahme       | 0,12 A               | 0,30 A               |
| Max. Leistungsaufnahme   | 1,44 W               | 3,6 W                |
| Anschluss                | 4-Pin PWM            | 4-Pin PWM            |
| Tacho-Impulse            | 2 Impulse / Umdrehung| 2 Impulse / Umdrehung|

### Pin-/Aderbelegung (Noctua-Standard)

| Pin | Farbe   | Funktion              | Richtung (aus ESP32-Sicht) |
| --- | ------- | --------------------- | -------------------------- |
| 1   | Schwarz | GND                   | Masse (gemeinsam)          |
| 2   | Gelb    | +12 V (Power)         | Versorgung vom Netzteil    |
| 3   | Grün    | Tacho / Sense (RPM)   | Eingang (Open-Collector)   |
| 4   | Blau    | PWM-Steuersignal      | Ausgang                    |

> Wichtig: Der **Leistungspfad** des Lüfters benötigt **12 V**. Der Lüfter darf nicht
> aus 3,3 V oder 5 V versorgt werden – er würde nicht bzw. nicht spezifikationsgemäß
> anlaufen.

## 2. Intel "4-Wire PWM Controlled Fans Specification"

Diese Spezifikation definiert das Verhalten der Pins 3 (Tacho) und 4 (PWM).

- **PWM-Frequenz:** Zielwert **25 kHz**, zulässiger Bereich **21–28 kHz**.
- **PWM-Steuereingang (Pin 4):** TTL-Pegel mit **interner Pull-up im Lüfter** zu 5 V –
  in neueren Konstruktionen auch zu **3,3 V**. Der V_IH-Schwellwert liegt typ. bei
  ~2,8 V. Ein **3,3-V-Logiksignal des ESP32 ist daher zur Ansteuerung ausreichend**
  (siehe Frage zur 3,3-V-Regelung weiter unten).
- **Tacho-Ausgang (Pin 3):** **Open-Collector**, benötigt einen **externen Pull-up**.
  Liefert 2 Impulse pro Umdrehung.
- Bei abgeklemmtem PWM-Signal läuft der Lüfter mit maximaler Drehzahl (Fail-Safe).

### Kann der Lüfter mit 3,3 V geregelt werden?

Ja – aber es muss zwischen **Versorgung** und **Steuersignal** unterschieden werden:

- **Versorgungsspannung (Pin 2):** **Nein.** Muss 12 V sein.
- **PWM-Steuersignal (Pin 4):** **Ja.** Das 25-kHz-PWM-Signal darf mit dem
  3,3-V-GPIO-Pegel des ESP32 erzeugt werden, da der V_IH-Schwellwert (~2,8 V) darunter
  liegt und der Lüfter intern selbst hochzieht. Ein Pegelwandler auf 5 V ist optional
  für mehr Störabstand, **technisch aber nicht erforderlich**.
- **Tacho (Pin 3):** Pull-up gegen **3,3 V** (nicht 5 V/12 V!), damit der
  ESP32-Eingang nicht überlastet wird.

## 3. Temperatursensor: DS18B20 (empfohlen)

Digitaler 1-Wire-Temperatursensor, ideal für diese Anwendung.

| Parameter            | Wert                         |
| -------------------- | ---------------------------- |
| Versorgungsspannung  | 3,0–5,5 V (hier **3,3 V**)   |
| Schnittstelle        | 1-Wire (digital)             |
| Messbereich          | -55 °C … +125 °C             |
| Genauigkeit          | ±0,5 °C (-10 … +85 °C)       |
| Pull-up Datenleitung | 4,7 kΩ gegen 3,3 V           |

Vorteile gegenüber Analogsensoren: digitale Übertragung (störarm), kein ADC nötig,
mehrere Sensoren an einem Bus möglich, direkte 3,3-V-Kompatibilität.

## 4. Mikrocontroller: ESP32 DevKitC v4 (ESP32-WROOM-32)

- GPIO-Logikpegel: **3,3 V** (NICHT 5-V-tolerant).
- LEDC-Hardware-PWM-Peripherie: bis 16 Bit Auflösung, Frequenz frei wählbar →
  25 kHz problemlos.
- 5V-Pin (VIN/5V) und 3V3-Pin am Board verfügbar.

## Referenzen

- Intel 4-Wire PWM Fan Spec: <https://glkinst.com/cables/cable_pics/4_Wire_PWM_Spec.pdf>
- Intel-4-Wire-Erläuterung: <https://www.pavouk.org/hw/fan/en_fan4wire.html>
- Noctua NF-F12 industrialPPC: <https://noctua.at/en/nf-f12-industrialppc-2000-pwm>
- DS18B20-Datenblatt: Maxim/Analog Devices DS18B20
