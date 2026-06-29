#pragma once

#include <cstddef>
#include <cstdint>

// Kapselt ein OTA-Update (Firmware oder Dateisystem) über die ESP32-Update-API.
// Einzige Verantwortung: einen Strom von Upload-Blöcken in die inaktive
// OTA-Partition schreiben und das Update abschließen.
class OtaUpdater {
 public:
  // Verarbeitet einen Upload-Block. index == 0 startet das Update,
  // isFinal schließt es ab. filesystem == true aktualisiert das LittleFS-Image
  // (statt der Firmware). Liefert false, sobald ein Fehler auftritt.
  bool processChunk(uint8_t* data, size_t length, size_t index, bool isFinal,
                    bool filesystem);

  bool isFinished() const;       // erfolgreich abgeschlossen
  bool hasError() const;
  const char* errorMessage() const;
  void reset();

 private:
  bool active = false;
  bool finished = false;
  bool failed = false;
  const char* errorText = "";
};
