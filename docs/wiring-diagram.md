# Anschluss-Schema: ESP32 DevKitC v4 → Noctua NF-F12 industrialPPC

## Pin-Zuordnung (Firmware-Konstanten)

| Funktion              | ESP32-GPIO | Hinweis                                   |
| --------------------- | ---------- | ----------------------------------------- |
| PWM-Steuersignal      | GPIO 25    | LEDC-Ausgang, 25 kHz, 3,3 V               |
| Tacho / RPM-Eingang   | GPIO 26    | Interrupt-Eingang, Pull-up gegen 3,3 V    |
| DS18B20 Daten (1-Wire)| GPIO 4     | Pull-up 4,7 kΩ gegen 3,3 V                |

## Komplettes Schema

```
              +12V DC Netzteil (z.B. 12V / 0,5A)
                   │ +12V              │ GND
                   │                   │
                   │            ┌──────┴───────────────────────┐
                   │            │      gemeinsame Masse (GND)   │
                   │            │                              │
   Noctua NF-F12   │            │                              │
   4-Pin Stecker   │            │                              │
   ┌───────────────┼────────────┼──────┐                       │
   │ Pin2 (gelb)  +12V ─────────┘      │                       │
   │ Pin1 (schw.)  GND ────────────────┼───────────────────────┤
   │ Pin4 (blau)   PWM ◄────────────┐  │                       │
   │ Pin3 (grün)   TACHO ──────┐    │  │                       │
   └────────────────────────── │ ── │ ─┘                       │
                               │    │                          │
                  10 kΩ        │    │                          │
        3V3 ──────/\/\/──────┐ │    │                          │
                             │ │    │                          │
   ESP32 DevKitC v4          │ │    │                          │
   ┌─────────────────────────┼─┼────┼──────────────────────────┤
   │ GPIO26 (Tacho-In) ◄─────┴─┘    │                          │
   │ GPIO25 (PWM-Out) ──────────────┘                          │
   │ GND ──────────────────────────────────────────────────────┘
   │ 3V3 ─────┬──────────────────────────────┐
   │          │                              │
   │          │  4,7 kΩ                      │
   │          └───/\/\/───┐                  │  DS18B20
   │ GPIO4 (1-Wire) ◄──────┴───────────────── DQ (Daten, gelb)
   │ 3V3 ───────────────────────────────────── VDD (rot)
   │ GND ───────────────────────────────────── GND (schwarz)
   └───────────────────────────────────────────────────────────
```

## Wichtige Regeln

1. **Gemeinsame Masse:** ESP32-GND und 12-V-Netzteil-GND **müssen verbunden** sein,
   sonst hat das PWM-Signal kein gemeinsames Bezugspotential und der Lüfter reagiert
   nicht zuverlässig.
2. **Lüfter-Power = 12 V:** Pin 2 ausschließlich an 12 V, niemals an 3V3/5V des ESP32.
3. **PWM-Signal = 3,3 V direkt:** GPIO25 geht direkt auf Pin 4 (blau). Kein Pegel-
   wandler nötig (V_IH des Lüfters ~2,8 V < 3,3 V). Optional: Levelshifter auf 5 V für
   mehr Störabstand bei langen Leitungen.
4. **Tacho-Pull-up gegen 3,3 V:** Pin 3 (grün) niemals gegen 12 V oder 5 V hochziehen –
   der Open-Collector-Ausgang würde sonst 12 V/5 V auf den 3,3-V-Eingang legen und den
   ESP32 zerstören. 10 kΩ gegen 3V3 ist korrekt.
5. **DS18B20:** 4,7 kΩ Pull-up zwischen DQ (GPIO4) und 3V3. Versorgung aus 3V3.

## Stückliste (BOM)

- 1× ESP32 DevKitC v4 (ESP32-WROOM-32)
- 1× Noctua NF-F12 industrialPPC-2000/3000 PWM
- 1× 12-V-DC-Netzteil (min. 0,5 A)
- 1× DS18B20 Temperatursensor (oder wasserdichte Kabelvariante)
- 1× Widerstand 4,7 kΩ (DS18B20-Pull-up)
- 1× Widerstand 10 kΩ (Tacho-Pull-up)
- Steckbrett / Lötplatine, Kabel, ggf. 4-Pin-Lüfter-Buchse
