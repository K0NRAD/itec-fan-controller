#include "OtaUpdater.h"

#include <Arduino.h>
#include <Update.h>

bool OtaUpdater::processChunk(uint8_t* data, size_t length, size_t index,
                              bool isFinal, bool filesystem) {
  if (index == 0) {
    reset();
    const int command = filesystem ? U_SPIFFS : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, command)) {
      failed = true;
      errorText = Update.errorString();
      return false;
    }
    active = true;
  }

  if (!active || failed) {
    return false;
  }

  if (Update.write(data, length) != length) {
    Update.abort();
    failed = true;
    errorText = Update.errorString();
    return false;
  }

  if (isFinal) {
    if (!Update.end(true)) {
      failed = true;
      errorText = Update.errorString();
      return false;
    }
    active = false;
    finished = true;
  }
  return true;
}

bool OtaUpdater::isFinished() const { return finished; }

bool OtaUpdater::hasError() const { return failed; }

const char* OtaUpdater::errorMessage() const { return errorText; }

void OtaUpdater::reset() {
  active = false;
  finished = false;
  failed = false;
  errorText = "";
}
