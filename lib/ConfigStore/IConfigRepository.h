#pragma once

#include "FanConfig.h"

// Repository-Abstraktion für die Persistenz der Konfiguration (DIP).
// Ermöglicht austauschbare Backends (NVS, Datei, Mock im Test).
class IConfigRepository {
 public:
  virtual ~IConfigRepository() = default;

  // Lädt die gespeicherte Konfiguration; liefert Default-Werte, falls noch
  // nichts gespeichert wurde.
  virtual FanConfig load() = 0;

  // Persistiert die Konfiguration. Liefert true bei Erfolg.
  virtual bool save(const FanConfig& config) = 0;
};
