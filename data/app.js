"use strict";

const STATUS_POLL_INTERVAL_MS = 2000;

const form = document.getElementById("config-form");
const tempSettings = document.getElementById("temp-settings");
const tempEnabled = document.getElementById("temperatureControlEnabled");
const feedback = document.getElementById("save-feedback");

// Verknüpft Range-Inputs mit ihrer Wertanzeige (<output>).
const sliderOutputs = [
  ["baseSpeedPercent", "baseSpeedOut"],
  ["thresholdTempC", "thresholdOut"],
  ["maxTempC", "maxTempOut"],
  ["maxSpeedPercent", "maxSpeedOut"],
];

function bindSliderOutputs() {
  for (const [inputId, outputId] of sliderOutputs) {
    const input = document.getElementById(inputId);
    const output = document.getElementById(outputId);
    const sync = () => {
      output.textContent = input.value;
    };
    input.addEventListener("input", sync);
    sync();
  }
}

function updateTempSettingsVisibility() {
  tempSettings.hidden = !tempEnabled.checked;
}

function applyConfigToForm(config) {
  document.getElementById("baseSpeedPercent").value = config.baseSpeedPercent;
  document.getElementById("thresholdTempC").value = config.thresholdTempC;
  document.getElementById("maxTempC").value = config.maxTempC;
  document.getElementById("maxSpeedPercent").value = config.maxSpeedPercent;
  tempEnabled.checked = config.temperatureControlEnabled;
  updateTempSettingsVisibility();
  for (const [inputId, outputId] of sliderOutputs) {
    document.getElementById(outputId).textContent =
      document.getElementById(inputId).value;
  }
}

function readConfigFromForm() {
  return {
    baseSpeedPercent: Number(document.getElementById("baseSpeedPercent").value),
    temperatureControlEnabled: tempEnabled.checked,
    thresholdTempC: Number(document.getElementById("thresholdTempC").value),
    maxTempC: Number(document.getElementById("maxTempC").value),
    maxSpeedPercent: Number(document.getElementById("maxSpeedPercent").value),
  };
}

function setFeedback(message, isError) {
  feedback.textContent = message;
  feedback.className = "feedback " + (isError ? "err" : "ok");
}

async function loadConfig() {
  try {
    const response = await fetch("/api/config");
    if (!response.ok) throw new Error(response.statusText);
    applyConfigToForm(await response.json());
  } catch (error) {
    setFeedback("Konfiguration konnte nicht geladen werden.", true);
  }
}

async function saveConfig(event) {
  event.preventDefault();
  setFeedback("Speichere ...", false);
  try {
    const response = await fetch("/api/config", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(readConfigFromForm()),
    });
    const result = await response.json();
    if (response.ok && result.saved) {
      applyConfigToForm(result.config);
      setFeedback("Gespeichert.", false);
    } else {
      setFeedback("Speichern fehlgeschlagen.", true);
    }
  } catch (error) {
    setFeedback("Speichern fehlgeschlagen.", true);
  }
}

async function pollStatus() {
  try {
    const response = await fetch("/api/status");
    if (!response.ok) return;
    const status = await response.json();
    document.getElementById("status-duty").textContent = status.dutyPercent;
    document.getElementById("status-temp").textContent =
      status.temperatureValid
        ? status.temperatureC.toFixed(1) + " °C"
        : status.sensorEnabled
          ? "kein Sensor"
          : "deaktiviert";
  } catch (error) {
    /* Netzwerkfehler ignorieren, nächster Poll versucht es erneut. */
  }
}

// --- OTA-Firmware-Update ---------------------------------------------------

const otaForm = document.getElementById("ota-form");
const otaFile = document.getElementById("ota-file");
const otaPass = document.getElementById("ota-pass");
const otaProgress = document.getElementById("ota-progress");
const otaFeedback = document.getElementById("ota-feedback");

function setOtaFeedback(message, isError) {
  otaFeedback.textContent = message;
  otaFeedback.className = "feedback " + (isError ? "err" : "ok");
}

function uploadFirmware(event) {
  event.preventDefault();
  const file = otaFile.files[0];
  if (!file) {
    setOtaFeedback("Bitte eine .bin-Datei auswählen.", true);
    return;
  }

  const request = new XMLHttpRequest();
  request.open("POST", "/update");
  // Basic-Auth (Benutzer "admin"); das Passwort stammt aus dem Eingabefeld.
  request.setRequestHeader("Authorization", "Basic " + btoa("admin:" + otaPass.value));

  request.upload.onprogress = (progressEvent) => {
    if (progressEvent.lengthComputable) {
      const percent = Math.round((progressEvent.loaded / progressEvent.total) * 100);
      otaProgress.value = percent;
      setOtaFeedback("Upload " + percent + " %", false);
    }
  };

  request.onload = () => {
    if (request.status === 200) {
      setOtaFeedback("Update erfolgreich – Gerät startet neu ...", false);
    } else if (request.status === 401) {
      setOtaFeedback("Falsches OTA-Passwort.", true);
    } else {
      setOtaFeedback("Update fehlgeschlagen.", true);
    }
  };

  // Beim Neustart bricht die Verbindung ab – das ist nach Erfolg normal.
  request.onerror = () => {
    setOtaFeedback("Verbindung verloren (vermutlich Neustart nach Update).", false);
  };

  const formData = new FormData();
  formData.append("firmware", file, file.name);
  setOtaFeedback("Upload startet ...", false);
  request.send(formData);
}

bindSliderOutputs();
tempEnabled.addEventListener("change", updateTempSettingsVisibility);
form.addEventListener("submit", saveConfig);
otaForm.addEventListener("submit", uploadFirmware);
loadConfig();
pollStatus();
setInterval(pollStatus, STATUS_POLL_INTERVAL_MS);
